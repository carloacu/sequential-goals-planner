(define (problem move-pickup-problem)
  (:domain move-pickup)

  (:objects
    robot1 - robot
    locationA locationB - location
    box1 - object
  )

  (:init
    (at robot1 locationA)
    (at-object box1 locationB)
  )

  (:goal
    (at-object box1 locationA)
  )
)
