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
* `TVirtualMCApplcation`,
* `TVirtualMCConcurrentApplcation`,

The actual interface to an MC is written by deriving from `TVirtualMC` (see [below][## Writing an interface] on how to do that) including another derivation from `TVirtualMCSensitiveDetector` for the interface to sensitive detectors.

An application or a framework using the interfaces need to provide their derivations of `TVirtualMCDecayer`, `TVirtualMCStack` and `TVirtualMCApplcation` or `TVirtualMCConcurrentApplcation` (when 2 or more engines should be used concurrently simulating different parts of the geometry), where an object of the latter will serve as the steering interface for running the entire machinery (see [below][## Writing an application] on how to do that)

Three further management classes are provided as singletons are provided, namely
* `TMCManager`,
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
The interfaces are no stand-alone classes but need interaction with the user code. This is especially necessary when the user wants to extract information for further processing during particle transport. E.g. in case a hit in a sensitive detector should be processed, a user routine is necessary since the transport code itself has no information on how to do that.

#### Required hooks to be called by the interface
* At each step of the engine: `TVirtualMCApplication::Stepping`


## Writing an application

Two scenarios are considered:
1. Only one transport engine is used for the entire geometry: In this case use `TVirtualMCApplication`
2. Two or more engines are implemented responsible for different parts of the geometry, different particles, phase space or any other decision defined by the user: In this case use `TVirtualMCConcurrentApplication`

### Required methods to be overriden
Both classes `TVirtualMCApplication` and `TVirtualMCConcurrentApplication` are pure virtual and the following methods need to be overriden
* `TVirtualMC[Concurrent]Application::ConstructGeometry()` (independent of the application type)
* `TVirtualMC[Concurrent]Application::GeneratePrimaries()` (independent of the application type)
* `TVirtualMC[Concurrent]Application::InitGeometry[Concurrent]()`
* `TVirtualMC[Concurrent]Application::BeginEvent[Concurrent]()`
* `TVirtualMC[Concurrent]Application::FinishEvent[Concurrent]()`
* `TVirtualMC[Concurrent]Application::BeginPrimary[Concurrent]()`
* `TVirtualMC[Concurrent]Application::FinishPrimary[Concurrent]()`
* `TVirtualMC[Concurrent]Application::PreTrack[Concurrent]()`
* `TVirtualMC[Concurrent]Application::PostTrack[Concurrent]()`
* `TVirtualMC[Concurrent]Application::Stepping[Concurrent]()`
depending on whether `TVirtualMCApplication` or `TVirtualMCConcurrentApplication` is used.

In most cases the virtual functions of `TVirtualMCApplication` are implemented in `TVirtualMCConcurrentApplication` wrapping its corresponding virtual methods and adding some further functionality to guarantee the correct behaviour of concurrently running transport engines. In that way the single `TVirtualMC` implementations always call the methods of `TVirtualMCApplication`. One main reason of providing 2 base classes for the application is the method `TVirtualMC[Concurrent]Application::Stepping[Concurrent]()` which is called at each step and is hence critical for the overall performance. In that way no run-time overhead is added when only one engine is used.

`ConstructGeometry()` and `GeneratePrimaries()` have the behaviour in any case.
