# Parsing

Kotaka's parser takes english input and translates it into commands and actions

## Parts of speech

The first concept is parts of speech which the sentence is divided into.

We do not implement adverbs yet.

### Verb

The verb is the first word of the sentence.  It determines how the rest
of the statement will be interpreted.

### Noun phrase

A noun phrase is something intended to identify a thing, and includes a
noun, potential adjectives, and potentially other things such as
ordinals, cardinals, or articles, and may also have possessives.

Nouns can be singular or plural.

### Preposition

A noun phrase can optionally be preceded by a preposition.

## Mapping

Once the statement is successfully parsed, the noun phrases are mapped to roles.

The verb provides a list of accepted roles and which prepositions are
acceptable for each role.

## Binding

Once the roles are mapped, each role is bound to the thing it's supposed to refer to.

### Base

The basis of binding is to match a noun phrase to a thing.

We first establish the intent of the phrase, whether the thing in
question is a specific thing, one thing among multiple candidates, or
multiple things.

This intent is determined by the usage of articles and ordinals, and
whether the noun in question is a singular noun or plural noun.

A single thing is determined by a singular noun.  A specific thing uses a
definite article or an ordinal, while a general thing uses an indefinite
article, and is the default when no article is used.

* the sword
* the third rock

* an apple
* a rock

Multiple things are determined by a plural noun, with an optional definite article.

The quantity can also be limited with a cardinal, but if a definite
article is used the quanity must completely match the available
candidates.

* three apples
* the rocks
* the three bananas

### Possessive

It's possible that one thing may be identified as a possessive of another
thing, which may require iterative binding.

The possessor is resolved first, and then the possessed thing is resolved
in the context of the possessor.

* my apple
* Amaterasu's fur
* the third tree's sixth stick
* the fourth dog's second paw's fifth toe

### Preposition

It's possible that we need to resolve a prepositional relation where the
scope of the phrase before the preposition depends on the phrase after the preposition.

* the coin in the purse
* the key on the book on the shelf
* the fifth apple in the third bowl on the second table
* the second diamond in sam's pouch

Hopefully we can use both prepositions and possessives.

### Override

Some verbs may want to override the default search process for binding a
phrase, sometimes binding one role may require reference to another role
that has already been bound.

Cases where a role's resolution depends on another role will somehow
require a way to specify the order in which to resolve roles.

* casting a spell from the mage's mental library of spells
* taking an item from a container, which requires the container to be resolved first.
* Other situations that require intervention versus the default
