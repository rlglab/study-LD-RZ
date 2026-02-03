# A Study of Solving Life-and-Death Problems in Go Using Relevance-Zone Based Solvers

This is the official repository of the IEEE ToG paper [A Study of Solving Life-and-Death Problems in Go Using Relevance-Zone Based Solvers](https://rlg.iis.sinica.edu.tw/papers/study-LD-RZ).

If you use this work for research, please consider citing our paper as follows:
```
@article{shih2025study,
  title={A Study of Solving Life-and-Death Problems in Go Using Relevance-Zone Based Solvers},
  author={Shih, Chung-Chin and Wu, Ti-Rong and Wei, Ting Han and Hsu, Yu-Shan and Guei, Hung and Wu, I-Chen},
  journal={IEEE Transactions on Games},
  year={2025},
  publisher={IEEE}
}
```
## Overview

This repository contains the **Relevance-Zone based solvers** for Go life-and-death problems, as described in the paper.

The following sections provide instructions on:
* **Build**: How to set up and compile the project.
* **Usage**: How to provide Tsumego problems as input.
* **Result**: How to interpret the results and the generated solution trees.
  
---
## Build

Follow the steps below to set up the environment and compile the project.

### 1. Environment Setup
First, clone the repository:

```bash
git clone git@github.com:rlglab/study-LD-RZ.git
cd study-LD-RZ
```

Then, start the container. 

This will automatically pull the necessary image (which may take a few minutes).

```bash
# Start the container
./scripts/run-container.sh
```

### 2. Compilation
Inside the container, run the following commands to clean, configure, and compile the solvers.

```bash
# Clean previous builds
./scripts/clean-up.sh

# Configure with CMake 
./scripts/setup-cmake.sh release caffe2

# Compile (optionally add -j <cores> for multithreaded compilation)
make
```

---

## Usage

To solve the Life-and-Death (L&D) problems, the solver must be provided with the directory where the problems are located (the default directory is `tsumego`.)

Each problem is encapsulated in **JSON format**, specifying problem settings such as sgf, ko rules, the player to move, crucial stones, and other configurations required by the solver.

The problems are specified in the `candidate.list` file, which already contains the seven analyzed problems. 

You can also solve other problems in the `tsumego` directory by editing `candidate.list`.

To apply the different settings for the **Transposition Table** and **Pattern Table** as described in the paper, run the following commands using the corresponding configuration files:

**For RZS-TT:**
```bash
Release/CGI -conf_file cfg/RZS-TT.cfg -mode tsumego_solver
```

**For RZS-PT:**
```bash
Release/CGI -conf_file cfg/RZS-PT.cfg -mode tsumego_solver
```

## Result

All results are saved in **JSON format** along with a solution tree (.sgf) under the `result` directory.

**Example:**
  - For the problem `chao_vol1_p088`:
  - The output files will be:
    ```
    result/result_chao_vol1_p088.json
    result/uct_tree_chao_vol1_p088.sgf
    ```
The result JSON file contains statistics generated from the solving process, such as `NumSimulations`, `Time`, and other relevant metrics.

The solution tree (.sgf) flie can be viewed using our specialized SGF viewer (Windows only), which supports board coloring for better visualization ([Download](https://rlg.iis.sinica.edu.tw/papers/study-LD-RZ/assets/editor.exe)).
  - **Green** indicates the Relevance-Zone (RZone).
  - **Red** indicates the winning move for the player attempting to live.
