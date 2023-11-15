# World state conditions


## What is a world state condition?

A world state condition is an expression that can be `true` or `false` according to a world state.


## Different kinds of conditions


### Simple fact


It can be

```
engaged_with_user
```

Note that the different between a fact and an event is the prefix `~event~`.

The condition is true when the fact `engaged_with_user` is in the world state.\
The condition is false otherwise.


### Punctual fact

It can be

```
~punctual~factName
```

The condition is punctually true when the fact `~punctual~factName` is raised.\
We identify a fact as punctual thanks to its prefix `~punctual~`.\
The condition is false the rest of the time.


### A negated fact


It can be

```
!fact
```

The condition is false when the fact `fact` is in the world state.\
The condition is true otherwise.



### A list of facts


It can be

```
fact1 & !fact2
```

The condition is true if all the sub-conditions of the list are true (so here if `fact1` is in the world state and if `fact2` is not in the world state).\
The condition is false otherwise.




### A fact with a value


It can be

```
fact=val
```

The condition is true when the fact `fact=val` is in the world state.\
The condition is false otherwise.


### A fact without a value


It can be

```
fact!=val
```

The condition is true when the fact `fact=dede` is in the world state for example (because `val` is different to `dede`).\
The condition is false otherwise.


### A fact without an empty value


It can be

```
fact!=
```

The condition is true when the fact `fact=val` or `fact=dede` is in the world state for example.\
The condition is false if the fact `fact` is not present in the world or if `fact` is present in the world state but without value (because "without value" is represented as an empty string for the value).



### Equality of values

Equality of fact values is done with a function `equals`

```
equals(fact1, fact2)
```

The condition is true if the value of `fact1` is equal to the value of `fact2` in the world.\
For example it is true if the world contains both `fact1=toto` and `fact2=toto` (they both have the value `toto`)


It can also be an equality comparison with an addition.

```
equals(number1, number2 + 3)
```

The condition is true if the world contains both `number1=4` and `number2=1` for example.




