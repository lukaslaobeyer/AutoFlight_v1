# Adding Drone Support in AutoFlight

Currently AutoFlight supports only the AR.Drone 2.0. The program is therefore undergoing a minor rewrite to make Drone support modular and to facilitate adding support for different Drones in a generic manner.

## The AFDrone class

The AFDrone class (see ``drone/afdrone.h``) is meant to be a partially abstract class from which new Drone controllers should inherit (therefore maintaining a common interface with the rest of AutoFlight).
