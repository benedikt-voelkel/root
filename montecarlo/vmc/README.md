# The Virtual Monte Carlo (VMC) Package

## Overview

The VMC package provides an abstract interface for Monte Carlo transport engines. Interfaces are implemented for
* [GEANT3](https://github.com/vmc-project/geant3)
* [GENAT4](https://github.com/vmc-project/geant4_vmc)
and the main interface derives from `TVirtualMC` implementing all necessary methods.

Before a user can instantiate an engine, an object deriving from `TVirtualMCApplication` needs to be present which has to be implemented by the user. It contains necessary hooks called from the `TVirtualMC`s depending on their internal state. At the same time it provides the bridge between the user code and VMC. For instance, the user code can contain the geometry construction routines somewhere which should be called from the implemented `UserApplication::ConstructGeometry()`.

More information can be found [here](https://root.cern.ch/vmc)

## Running multiple different engines

The simulation of an event can be share among multiple different engines which are handled by a singleton `TMCManager` object. In this case a user has to call `TVirtualMCApplication::RequestMCManager()` in the constructor of the user application. This creates the manager which is then available via the protected pointer `TVirtualMCApplication::fMCManager` but can also be obtained using the static method `TMCManager *TMCManager::Instance()`.

`TMCManager` provides further the following interfaces:

* `void SetUserStack(TVirtualMCStack* userStack)`: This notifies the manager on the user stack such that it will be kept up-to-date during the simulation. The `TMCManager` will abort the run if the user stack hasn't been set.
* `void ForwardTrack(Int_t toBeDone, Int_t trackId, Int_t parentId, TParticle* userParticle)`: The user is still the owner of all track objects being created (aka `TParticle`). Hence, all engine calls to `TVirtualMCStack::PsuhTrack(...)` are forwarded to the user stack. This can then invoke the `ForwardTrack(..)` method of the manager to pass the pointer to the constructed `TParticle` object. If a particle should be pushed to an engine other than the one currently running the engine's id has to be provided as the second (optional) argument.
* `void TransferTrack(Int_t targetEngineId)`: During a call to `TVirtualMCApplication::Stepping()` the user might decide that the current track should be transferred to another engine, for instance, if the entered volume should be simulated with another engine. Specifying the ID of the target engine the manager will take care of interrupting the track in the current engine, extracting the kinematics and geometry state and it will push this to the stack of the target engine.
* `void Apply(std::function<void(TVirtualMC *)> engineLambda)` will apply a function to all registered engines.
* `void Init(std::function<void(TVirtualMC *)> initFunction)` will apply the specified function to all registered engines and performs further internal initialization steps such as checking for a registered user stack etc.
* `void Run(Int_t nEvents)` steers a run for a specified number of events.
* `void ConnectEnginePointer(TVirtualMC *&mc)` gives the possibility for a user to pass a pointer which will always be set to point to the current engine.
* `TVirtualMC *GetCurrentEngine()` provides the user with the currently running engine.

An example of how the `TMCManager` is utilized in a multi-run can be found in `examples/EME` of the [GEANT4_VMC repository](https://github.com/vmc-project/geant4_vmc).

### Workflow

**Implementation**
1. Implement your application as you have done before. Request the `TMCManager` in your constructor if needed via `TVirtualMCApplication::RequestMCManager()`
2. Implement your user stack as you have done before. At an appropriate place (e.g. in `UserStack::PushTrack(...)`) you should call `TMCManager::ForwardTrack(...)` to forward the pointer to your newly constructed `TParticle` objects.
3. Set your stack using `TMCManager::SetUserStack(...)`

**Usage**
1. Instantiate your application
2. Instantiate the engines you want to use (if not done within your application)
3. Call `TMCManager::Init(...)`, optionally with a lambda in the argument (can also be wrapped into custom method you implemented in your application)
4. Call `TMCManager::Run(...)` (can also be wrapped into custom method you implemented in your application)

**Further comments**

The geometry is built once centrally via the `TMCManager` calling

1. `TVirtualMCApplication::ConstructGeometry()`
2. `TVirtualMCApplication::MisalignGeometry()`
2. `TVirtualMCApplication::ConstructOpGeometry()`

so it is expected that these methods do not depend on any engine in case of a multi-run.

If multiple engines have been instantiated never call `TVirtualMC::ProcessRun(...)` on the engine since that would bypass the `TMCManager`
