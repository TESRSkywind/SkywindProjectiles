# New projectiles documentation

Settings could be reloaded by Numpad7 key in game!

## Root

It has single [FormIDs map](#formids) and [ModConfig](#modconfig) item for every mod you want to change.

Example:

```json
{
    "FormIDs": { ... },      // (OPT) Define FormIDs map
    "Skyrim.esm": { ... },   // Configure projectiles in Skyrim.esm
    "FenixMod.esp": { ... }  // Configure projectiles in FenixMod.esp
}
```

### FormIDs

A map: string -> [Plugin formID](#plugin-formid) for more convenience.  
Keys are started with `key_`. It is useful to store and use everywhere `"key_fireballSpell"` instead of `"Skyrim.esm|0x1C789"`.

Example:

```json
"FormIDs": {
    // remember fireball spell and projectile  IDs for further using
    "key_spellFireball": "Skyrim.esm|0x1C789",
    "key_projFireball": "Skyrim.esm|0x10FBED"
}
```

### ModConfig

Besides `"FormIDs"` key, [Root](#root) also may have a key for every plugin where you want to [change projectiles](#new-types). There are 3 optional keys: [Homing](#homing), [Multicast](#multicast) and [Emitters](#emitters).

Example:

```json
"Skyrim.esm": {
    "Homing": { ... },     // (OPT) Configure Homing projectiles
    "Multicast": { ... },  // (OPT) Configure Multicast projectiles
    "Emitters": { ... }    // (OPT) Configure Emitters projectiles
}
```

## New types

There are 3 types: [Homing](#homing), [Multicast](#multicast) and [Emitters](#emitters).

### Homing

Projectiles that can follow to the specific target.

Example:

```json
...
"Homing": {
    ...
    // make it has const acceleration 5, search any nearest target
    // and ignore all this if casted by NPC
    //   (i.e. for player it is homing, for NPC it is regular)
    "0x800": {  // key as number
        "type": "ConstAccel",
        "target": "Nearest",
        "acceleration": 5,
        "caster": "Player"       // (OPT)
    },
    // make it has const speed with rotation time 3.5 sec
    // search nearest to cursor target within 10 degs
    "key_coolProj": {  // key as map entry
        "type": "ConstSpeed",
        "rotationTime": 3.5,
        "target": "Cursor",
        "cursorAngle": 10      // required only for target = Cursor
    },
    ...
}
...
```

This is a map with [these](#keys-of-new-types) keys and json objects as values. Each object represent a homing projectile type.

There are two implementations of trajectories (use `type` property).

* `ConstSpeed` -- projectile has constant linear speed.  
It is parametrized by `rotationTime` property. This is time needed to rotate at 180 deg (5, 3.5, 2.5 values are pretty nice).
* `ConstAccel` -- projectile has constant acceleration vector length.  
It is parametrized by `acceleration` property. This is acceleration vector length (5, 10, 50 values are pretty nice).

Example:

```json
...
// make it have constant acceleration 5
"type": "ConstAccel",
"acceleration": 5,
...
```

Ways to obtain target (use property `target`):

* `Nearest` -- find nearest to caster target.
* `Hostile` -- find nearest aggressive to caster target.
* `Cursor` -- find nearest to the player cursor target.

```json
...
// make it homing to the nearest actor
"target": "Nearest",
...
```

It is possible to make homing either only for NPC or Player (use *caster* property):

* `Both` -- homing effect applies for both casters (default).
* `NPC` -- homing effect applies only for NPC casters.
* `Player` -- homing effect applies only for Player caster.

```json
...
// make it homing only when launched by NPC, not Player
"caster": "NPC",  // (OPT)
...
```

### Multicast

Projectiles that can spawn multiple projectiles when casted.

Example:

```json
...
"Multicast": {
    ...
    "0x800": {
        // this is default settings for every spawn group
        // if the setting is not defined in the spawn group, it taken from here
        "spellID": "key_spellLightning",  // (OPT) cast lightning
        "shape": "Circle",       // (OPT) spawn circles
        "rotation": "Parallel",  // (OPT) cast parallel to cast direction
        "sound": "Single",       // (OPT) play sound once per group
        "count": 10,             // (OPT) 10 strikes please
        "spawnData": [  // An array of spawn groups
            // spawn horizontal circle pointed up
            // properties "shape", "rotation", "sound", "spellID", "count"
            //  are defined in default section.
            {
                // (OPT) pretend we're looking 90deg upper
                //  than we actually looking at
                "normal": [0,0,1],
                "size": 100         // (OPT) Circle size
            },
            // spawn scatter ice spikes
            {
                "count": 20,                     // (OPT) spawn 20 things
                "spellID": "key_spellIceSpike",  // cast ice spike
                "shape": "Single",               // spawn from the hand
                // horizontal spread is 5deg and vertical spread is 30deg
                "rotRnd": [30, 5],
                // move up our initial direction, to prevent ground shooting
                "rotDelta": [-25, 0],
                "NewProjType": [ ... ]           // (OPT)
            }
        ]
    },
    ...
},
...
```

This is a map with [these](#keys-of-new-types) keys and json objects as values. Each object represent a multicast projectile type.

There could be multiple spawn groups, defined in object's `spawnData` array of json objects property. Each group describes logically separate spawn unit. For example, the entire group used for single shape or single spell type.

The object also has some default properties that are used if no such property is set in the spawn group:

* `spellID` -- defines the spell will be casted for every projectile in the spawn group. It could be:
  * `Current` -- (default) take projectile's spell itself.
  * [FormID](#plugin-formid) or [Key](#formids) formIDs that should have SPEL record.

* `shape` -- define spawn shape. It could be:
  * `Single` -- (default) spawn all projectiles in a single spot.
  * `HorizontalLine` -- arrange projectiles in a horizontal line.
  * `VerticalLine` -- arrange projectiles in a vertical line.
  * `Circle` -- arrange projectiles in a circle.
  * `HalfCircle` -- arrange projectiles in a half of circle.
  * `FillSquare` -- arrange projectiles in filled square.

* `rotation` -- define how to rotate projectiles. It could be:
  * `Parallel` -- (default) all projectiles has same direction as cast direction.
  * `ToSight` -- make all projectiles in the group pointing at the point of sight.
  * `ToCenter` -- make all projectiles point to the center of the spawn group.
  * `FromCenter` -- make all projectiles point from the center of the spawn group.

* `sound` -- defines sound amount. It could be:
  * `Every` -- (default) play sound for every projectile.
  * `Single` -- play sound once for a group.
  * `None` -- do not play sounds for the group.

* `count` -- a number of projectiles to spawn in the group. Default: 1.

In addition to the above properties, spawn group have those:

* `size` -- defines the size of shape. Must be positive. Used only for non-Single shapes.
* `pos` -- defines spawn group center offset relative to cast position. Default: [0,0,0].
* `posRnd` -- maximal random offset. Default: [0,0,0].
* `normal` -- assume that the cast direction is along Y axis. Then this parameter defines cast direction for this spawn group. Default: [0,1,0], i.e. along Y axis too. It is not necessary to have it unitized.
* `rotDelta` -- rotate launched projectile to this angles. Default: [0,0].
* `rotRnd` -- maximal random angles offset. Default: [0,0].

### Emitters

Projectiles that can call some function while flying.

Example:

```json
...
"Skyrim.esm": {
    ...
    "Multicast": {
        ...
        "0xFF000000": { ... },
        ...
    },
    "Emitters": {
        "0x800": {
            "key": "0x01"
        }
    }
    ...
},
"Emitters": {
    ...
    // this projectile emits something every 0.5 sec of flying
    "0x01": {
        "type": "Multicast",  // multicast things
        "interval": 0.5,      // cast every 0.5 sec
        "key": "0xFF000000"   // key of multicast thing
    }
    ...
},
...
```

This is a map with keys from `0x01` to `0xFF` and json objects as values. Each object represent an emitter projectile type.

Emitter can call functions while flying. Use `interval` property to set calling interval and `type` property to set type of the function.  
Available functions are:

* Multicast -- perform multicast action. It uses `key` property to set the key of needed action. For this purpose "0xFFXXXXXX" used.

## Additional definitions

### Plugin formID

A pair (pluginName, formID) fully describes a form in the game. Used string format plugin|formID for this, e.g. **FenixMod.esp|0x31415**.

### Keys of new types

Every json object ([Homing](#homing), [Multicast](#multicast) and [Emitter](#emitters)) has a map of respective projectile config. When projectile is launching by the game, plugin tries to find the formID of its base (i.e. PROJ record in xEdit) in every map. If there is such a key in the map, the given projectile has this type.  
Each key could be one of follows:

* 8 or less digits hex number. It means either we are searching in the plugin which has this setting in or using [generic](#emitters) `0xFFXXXXXX` key.
* A key defined in [FormIDs](#formids) map.
