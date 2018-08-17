# Using the VMC package

The Virtual Monte Carlo (VMC) allows to run different simulation Monte Carlo without changing the user code and therefore the input and output format as well as the geometry and detector response definition.
Some code documentation can be found [here](https://root.cern.ch/vmc). The following sections give a more detailed description on the requirements when using the virtual classes for deriving a new interface for a Monte Carlo transport engine.

## Design

### Class overview

There are six pure virtual classes
* `TVirtualMC`,
* `TVirtualMCSensitiveDetector`,
* `TVirtualMCDecayer`,
* `TVirtualMCStack`,
* `TVirtualMCApplcation`.

The actual interface to an MC is written by deriving from `TVirtualMC` (see [below][## Writing an interface] on how to do that) including another derivation from `TVirtualMCSensitiveDetector` for the interface to sensitive detectors.

An application or a framework using the interfaces need to provide their derivations of `TVirtualMCDecayer`, `TVirtualMCStack` and `TVirtualMCApplcation`, where an object of the latter will serve as the steering interface for running the entire machinery (see [below][## Writing an application] on how to do that)

Three further management classes are provided as singletons are provided, namely
* `TMCHandler`,
* `TMCStackManager`
* `TMCStateManager`,
 which are used to guarantee a smooth run and a unified behaviour of all possible interfaces. To do so please note the requirements of how the classes are used in the implementation of an MC interface.

### General assumptions

The basic objects (with a physical interpretation) assumed to be accessible from an interface's back-end are (form smallest to largest granularity) are:
* a step: resolvable space-time positions of a particle during its transport
* a track: built by successive (time-wise) steps of a particle
* an event: made up of a certain set of tracks
* a run: consisting of a number of events




## Writing an interface

### Hooks and callbacks
The interfaces are no stand-alone classes but need interaction with the user code. This is especially useful when the user code relies on certain states, e.g. a track or a step, during particle transport to extract information for further processing. In case a hit in a sensitive detector should be processed, a user routine is necessary since the transport code itself has no information in how to do that.

#### Required hooks to be called by the interface
* At each step of the engine: `TVirtualMCApplication::Stepping`




## Writing an application
