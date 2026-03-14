# Negotiator Library

A decentralized power negotiation library designed for MADNESS' grid source nodes. This library allows multiple agents to converge on a shared power demand by distributing the load depending on their estimated reliability (covariance).

## Overview

The `Negotiator` library implements a **Consensus Balancing Algorithm**. Each source node in the network calculates its own power proposal based on the total demand and the "weights" of all active participants. 

### Consensus Balancing
Each node $i$ computes its proposed power $P_{prop, i}$ as follows:

$$P_{prop, i} = \left( \frac{w_i}{\sum w_{total}} \right) \cdot P_{demand}$$

Where the weight $w$ is the inverse of the node's covariance ($1/\sigma^2$). This ensures that:
1. **Reliability-based distribution**: Nodes with lower covariance (higher certainty) take on a larger portion of the load.
2. **Mathematical Balance**: The sum of all proposals naturally converges to the total demand without a central coordinator.

## Features

- **Decentralized Negotiation**: No master node required: agents communicate via MADS net.
- **Dynamic Load Tracking**: Automatically detects new loads or sources and recalculates the fair share.
- **Fault Tolerance**: Automatic "node cleaning" removes participants that haven't sent updates within a specified timeout (`TIME_SLEEP`).
- **Stability Detection**: Tracks local convergence and sets a stability flag once the proposal changes less than the defined threshold.
- **Ergodicity Check**: Monitors temporal stability of proposals to detect oscillations and apply an "ergodic penalty" to influence external filters (like an EKF).

## Installation

This library requires the [nlohmann/json](https://github.com/nlohmann/json) library for message parsing.

* Include `negotiator.hpp` and `negotiator.cpp` in the required MADS agent.
* Ensure you are using a C++17 compliant compiler and the last version of [MADS](https://github.com/pbosetti/MADS.git);
   Include the repo in the agent's `CMakeLists.txt` file as follow:
```
#
# Along with the other <FetchContent_Declare> sections
#

FetchContent_Declare(negotiator
  GIT_REPOSITORY https://github.com/pure-MADNESS/negotiator-lib.git
  GIT_TAG        main
  GIT_SHALLOW    TRUE
)

#
# ...
#

#
# After all the <FetchContent_Declare>
#

FetchContent_MakeAvailable(pugg json negotiator <otherlibraries>)

#
# ...
#

#
# Edit the <add_plugin> command
#

add_plugin(<agentname> LIBS Eigen3::Eigen negotiator <otherlibraries> SRCS ${SRC_DIR}/<agentname>.cpp)

```

## Usage

### Initialization
