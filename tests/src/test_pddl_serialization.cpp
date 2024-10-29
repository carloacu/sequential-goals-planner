#include "test_pddl_serialization.hpp"
#include <assert.h>
#include <iostream>
#include <contextualplanner/types/domain.hpp>
#include <contextualplanner/types/problem.hpp>
#include <contextualplanner/types/ontology.hpp>
#include <contextualplanner/types/predicate.hpp>
#include <contextualplanner/types/setofentities.hpp>
#include <contextualplanner/types/setofpredicates.hpp>
#include <contextualplanner/types/worldstatemodification.hpp>
#include <contextualplanner/util/serializer/deserializefrompddl.hpp>
#include <contextualplanner/util/serializer/serializeinpddl.hpp>

namespace
{
template <typename TYPE>
void assert_eq(const TYPE& pExpected,
               const TYPE& pValue)
{
  if (pExpected != pValue)
    assert(false);
}

template <typename TYPE>
void assert_true(const TYPE& pValue)
{
  if (!pValue)
    assert(false);
}

template <typename TYPE>
void assert_false(const TYPE& pValue)
{
  if (pValue)
    assert(false);
}


void _test_pddlSerializationParts()
{
  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromPddl("type1 type2 - entity");
  ontology.constants = cp::SetOfEntities::fromPddl("toto - type1\n"
                                                  "titi - type2", ontology.types);

  cp::Predicate pred("(pred_a ?e - entity)", true, ontology.types);
  assert_eq<std::string>("pred_a(?e - entity)", pred.toStr());

  {
    std::size_t pos = 0;
    ontology.predicates = cp::SetOfPredicates::fromPddl("(pred_a ?e - entity)\n"
                                                        "pred_b\n"
                                                        "(pred_c ?e - entity)\n"
                                                        "(battery-amount ?t - type1) - number", pos, ontology.types);
    assert_eq<std::string>("battery-amount(?t - type1) - number\n"
                           "pred_a(?e - entity)\n"
                           "pred_b()\n"
                           "pred_c(?e - entity)", ontology.predicates.toStr());
  }

  {
    std::size_t pos = 0;
    cp::Fact fact = cp::Fact::fromPddl("(pred_a toto)", ontology, {}, {}, pos, &pos);
    assert_eq<std::string>("(pred_a toto)", fact.toPddl(true));
    assert_eq<std::string>("pred_a(toto)", fact.toStr());
  }

  {
    std::size_t pos = 0;
    std::unique_ptr<cp::WorldStateModification> ws = cp::pddlToWsModification("(decrease (battery-amount toto) 4)", pos, ontology, {}, {});
    if (!ws)
      assert_true(false);
    assert_eq<std::string>("decrease(battery-amount(toto), 4)", ws->toStr());
  }

  {
    std::size_t pos = 0;
    std::unique_ptr<cp::WorldStateModification> ws = cp::pddlToWsModification("(forall (?e - entity) (when (pred_a ?e) (pred_c ?e))", pos, ontology, {}, {});
    if (!ws)
      assert_true(false);
    assert_eq<std::string>("forall(?e - entity, when(pred_a(?e), pred_c(?e)))", ws->toStr());
  }

}


void _test_loadPddlDomain()
{
  std::map<std::string, cp::Domain> loadedDomains;
  {
    auto firstDomain = cp::pddlToDomain(R"(
  (define
      (domain building)
      (:types ; a comment
          ; line with only a comment
          site material - object  ; a comment

          ; line with only a comment
          bricks cables windows - material
      )
      (:constants mainsite - site)

      (:predicates
          (walls-built ?s - site)
          (foundations-set ?s - site)  ; a comment
          (on-site ?m - material ?s - site)

          ; line with only a comment
          (material-used ?m - material)
      )

      (:action BUILD-WALL
          :parameters (?s - site ?b - bricks)   ; a comment
          :precondition
              (foundations-set ?s)  ; a comment
          :effect (walls-built ?s)   ; a comment
          ; :expansion ;deprecated
      )

      (:action BUILD-WALL-WITHOUT-CONDITION
          :parameters (?s - site ?b - bricks)   ; a comment
          :effect (walls-built ?s)   ; a comment
          ; :expansion ;deprecated
      )

  )", loadedDomains);
    loadedDomains.emplace(firstDomain.getName(), std::move(firstDomain));
  }

  auto domain = cp::pddlToDomain(R"(
(define
    (domain construction)
    (:extends building)
    (:requirements :strips :typing)
    (:types
        car
        ball
    )
    (:constants maincar - car)

    ;(:domain-variables ) ;deprecated

    (:predicates
        (cables-installed ?s - site)
        (held ?b - ball)
        (site-built ?s - site)
        (windows-fitted ?s - site)
    )

    (:functions;a comment
        (battery-amount ?r - car)
        (distance-to-floor ?b - ball)
        (velocity ?b - ball)  ; a comment
        (size ?b - ball)
    )

    (:timeless (foundations-set mainsite))

    ;(:safety
        ;(forall
        ;    (?s - site) (walls-built ?s)))
        ;deprecated

    (:axiom
        :vars (?s - site)
        :context (and
            (walls-built ?s)
            (windows-fitted ?s)
            (cables-installed ?s)
            (exists (?b - ball)
                (and
                    (= (velocity ?b) 7)
                    (= (size ?b) 10)
                )
            )

        )
        :implies (site-built ?s)
    )

    (:event HIT-GROUND
        :parameters (?b - ball)
        :precondition (and
            (not (held ?b))
            (<= (distance-to-floor ?b) 0)
            (> (velocity ?b) 0)
        )
        :effect (
            (assign (velocity ?b) (* -0.8 (velocity ?b)))
        )
    )

    (:durative-action BUILD-WALL-DURATIVE
        :parameters
          (?s - site ?b - bricks)

        :duration
            (= ?duration 1)

        :condition
          (and
            (at start (on-site ?b ?s))
            (at start (foundations-set ?s))
            (over all (not (walls-built ?s)))
            (at start (not (material-used ?b)))
          )

        :effect
          (and
            (at start (walls-built ?s))
            (at end (forall (?sa - site) (when (site-built ?sa) (walls-built ?sa))))
            (at end (material-used ?b))
            (at end (decrease (battery-amount maincar) 4))
          )
    )

)
)", loadedDomains);
  loadedDomains.emplace(domain.getName(), domain);

  const std::string expectedDomain = R"((define
    (domain construction)
    (:requirements :strips :typing)

    (:types
        site material - object
        bricks cables windows - material
        car
        ball
    )

    (:constants
        maincar - car
        mainsite - site
    )

    (:predicates
        (cables-installed ?s - site)
        (foundations-set ?s - site)
        (held ?b - ball)
        (material-used ?m - material)
        (on-site ?m - material ?s - site)
        (site-built ?s - site)
        (walls-built ?s - site)
        (windows-fitted ?s - site)
    )

    (:functions
        (battery-amount ?r - car) - number
        (distance-to-floor ?b - ball) - number
        (size ?b - ball) - number
        (velocity ?b - ball) - number
    )

    (:timeless
        (foundations-set mainsite)
    )

    (:event HIT-GROUND

        :parameters
            (?b - ball)

        :precondition
            (and
                (not (held ?b))
                (<= (distance-to-floor ?b) 0)
                (> (velocity ?b) 0)
            )

        :effect
            (assign (velocity ?b) (* (velocity ?b) -0.800000))
    )

    (:event from_axiom

        :parameters
            (?s - site)

        :precondition
            (and
                (walls-built ?s)
                (windows-fitted ?s)
                (cables-installed ?s)
                (exists (?b - ball) (and
                    (= (velocity ?b) 7)
                    (= (size ?b) 10)
                ))
            )

        :effect
            (site-built ?s)
    )

    (:event from_axiom_2

        :parameters
            (?s - site)

        :precondition
            (or
                (not (walls-built ?s))
                (not (windows-fitted ?s))
                (not (cables-installed ?s))
                (not (exists (?b - ball) (and
                    (= (velocity ?b) 7)
                    (= (size ?b) 10)
                )))
            )

        :effect
            (not (site-built ?s))
    )

    (:durative-action BUILD-WALL
        :parameters
            (?s - site ?b - bricks)

        :duration (= ?duration 1)

        :condition
            (at start (foundations-set ?s))

        :effect
            (at end (walls-built ?s))
    )

    (:durative-action BUILD-WALL-DURATIVE
        :parameters
            (?s - site ?b - bricks)

        :duration (= ?duration 1)

        :condition
            (and
                (at start (on-site ?b ?s))
                (at start (foundations-set ?s))
                (at start (not (material-used ?b)))
                (over all (not (walls-built ?s)))
            )

        :effect
            (and
                (at start (walls-built ?s))
                (at end (forall (?sa - site) (when (site-built ?sa) (walls-built ?sa))))
                (at end (material-used ?b))
                (at end (decrease (battery-amount maincar) 4))
            )
    )

    (:durative-action BUILD-WALL-WITHOUT-CONDITION
        :parameters
            (?s - site ?b - bricks)

        :duration (= ?duration 1)

        :effect
            (at end (walls-built ?s))
    )

))";

  auto outDomainPddl1 = cp::domainToPddl(domain);
  if (outDomainPddl1 != expectedDomain)
  {
    std::cout << outDomainPddl1 << std::endl;
    assert_true(false);
  }


  cp::DomainAndProblemPtrs domainAndProblemPtrs = pddlToProblem(R"((define
    (problem buildingahouse)
    (:domain construction)
    ;(:situation <situation_name>) ;deprecated
    (:objects
        s1 - site
        b - bricks
        w - windows
        c - cables
    )
    (:init
        (on-site b s1)
        (on-site c s1)
        (on-site w s1)
    )
    (:goal (and  ; a comment
            (walls-built s1)
            (cables-installed s1)
            (windows-fitted s1)
        )
    )
))", loadedDomains);


  std::string expectedProblem = R"((define
    (problem buildingahouse)
    (:domain construction)

    (:objects
        b - bricks
        c - cables
        s1 - site
        w - windows
    )

    (:init
        (on-site b s1)
        (on-site c s1)
        (on-site w s1)
    )

    (:goal
        (and
            (walls-built s1)
            (cables-installed s1)
            (windows-fitted s1)
        )
    )

    (:constraints
        (and ; These contraints are to specify the goals order
            (preference p0
                (sometime-after (walls-built s1) (cables-installed s1))
            )
            (preference p1
                (sometime-after (cables-installed s1) (windows-fitted s1))
            )
        )
    )

))";

  auto outProblemPddl1 = cp::problemToPddl(*domainAndProblemPtrs.problemPtr,
                                           *domainAndProblemPtrs.domainPtr);
  if (outProblemPddl1 != expectedProblem)
  {
    std::cout << outProblemPddl1 << std::endl;
    assert_true(false);
  }

  // deserialize what is serialized
  auto domain2 = cp::pddlToDomain(outDomainPddl1, {});
  std::map<std::string, cp::Domain> loadedDomains2;
  loadedDomains2.emplace(domain2.getName(), domain2);
  auto outDomainAndProblemPtrs2 = cp::pddlToProblem(outProblemPddl1, loadedDomains2);

  // re serialize
  auto outDomainPddl2 = cp::domainToPddl(domain2);
  if (outDomainPddl2 != expectedDomain)
  {
    std::cout << outDomainPddl2 << std::endl;
    assert_true(false);
  }

  auto outProblemPddl2 = cp::problemToPddl(*outDomainAndProblemPtrs2.problemPtr,
                                           *outDomainAndProblemPtrs2.domainPtr);
  if (outProblemPddl2 != expectedProblem)
  {
    std::cout << outProblemPddl2 << std::endl;
    assert_true(false);
  }
}

}




void test_pddlSerialization()
{
  _test_pddlSerializationParts();
  _test_loadPddlDomain();

  std::cout << "PDDL serialization is ok !!!!" << std::endl;
}
