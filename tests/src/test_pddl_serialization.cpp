#include "test_pddl_serialization.hpp"
#include <assert.h>
#include <iostream>
#include <contextualplanner/types/domain.hpp>
#include <contextualplanner/types/ontology.hpp>
#include <contextualplanner/types/predicate.hpp>
#include <contextualplanner/types/setofentities.hpp>
#include <contextualplanner/types/setofpredicates.hpp>


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
  ontology.types = cp::SetOfTypes::fromStr("type1 type2 - entity");
  ontology.constants = cp::SetOfEntities::fromStr("toto - type1\n"
                                                  "titi - type2", ontology.types);

  cp::Predicate pred("(pred_a ?e - entity)", true, ontology.types);
  assert_eq<std::string>("pred_a(?e - entity)", pred.toStr());

  {
    std::size_t pos = 0;
    ontology.predicates = cp::SetOfPredicates::fromPddl("(pred_a ?e - entity)\n"
                                                        "pred_b", pos, ontology.types);
    assert_eq<std::string>("pred_a(?e - entity)\n"
                           "pred_b()", ontology.predicates.toStr());
  }

  {
    std::size_t pos = 0;
    auto fact = cp::Fact::fromPDDL("(pred_a toto)", ontology, {}, {}, pos, &pos);
    assert_eq<std::string>("pred_a(toto)", fact.toStr());
  }

  std::cout << "PDDL serialization is ok !!!!" << std::endl;
}


void _test_loadPddlDomain()
{
  auto domain = cp::Domain::fromPddl(R"(
(define
    (domain construction)
    (:extends building)
    (:requirements :strips :typing)
    (:types
        site material - object
        bricks cables windows - material
    )
    (:constants mainsite - site)

    ;(:domain-variables ) ;deprecated

    (:predicates
        (walls-built ?s - site)
        (windows-fitted ?s - site)
        (foundations-set ?s - site)
        (cables-installed ?s - site)
        (site-built ?s - site)
        (on-site ?m - material ?s - site)
        (material-used ?m - material)
    )

    (:timeless (foundations-set mainsite))

    ;(:safety
        ;(forall
        ;    (?s - site) (walls-built ?s)))
        ;deprecated

    (:action BUILD-WALL
        :parameters (?s - site ?b - bricks)
        :precondition (and
            (on-site ?b ?s)
            (foundations-set ?s)
            (not (walls-built ?s))
            (not (material-used ?b))
        )
        :effect (and
            (walls-built ?s)
            (material-used ?b)
        )
        ; :expansion ;deprecated
    )

    (:axiom
        :vars (?s - site)
        :context (and
            (walls-built ?s)
            (windows-fitted ?s)
            (cables-installed ?s)
        )
        :implies (site-built ?s)
    )

    ;Actions omitted for brevity

    (:durative-action move
        :parameters
            (?r - rover
             ?fromwp - waypoint
             ?towp - waypoint)

        :duration
            (= ?duration 5)

        :condition
          (and
              (at start (rover ?rover))
              (at start (waypoint ?from-waypoint))
              (at start (waypoint ?to-waypoint))
              (over all (can-move ?from-waypoint ?to-waypoint))
              (at start (at ?rover ?from-waypoint))
              (at start (> (battery-amount ?rover) 8)))

        :effect
          (and
                (decrease (fuel-level ?t) (* 2 #t))
              (at end (at ?rover ?to-waypoint))
              (at end (been-at ?rover ?to-waypoint))
              (at start (not (at ?rover ?from-waypoint)))
              (at start (decrease (battery-amount ?rover) 8))
                (at end (increase (distance-travelled) 5))
                )
  )

)
)");

}

}




void test_pddlSerialization()
{
  _test_pddlSerializationParts();
  _test_loadPddlDomain();

  std::cout << "PDDL serialization is ok !!!!" << std::endl;
}
