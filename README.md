# usdosr

USDOS wrapped in Rcpp package for use in R.  Provides a single function `run_usdos` that takes a config file as input and runs the model, writing output to file.

## Installation

To install, either use R's devtools package 
```r
devtools::install_github("webblabb/usdosr")
```
or build the package yourself using these steps:
1. Download and unzip the repository
2. In a terminal window, navigate to the directory containing the package directory.
3. Type `R CMD build usdosr-master`. This will bring up several lines of checking, cleaning, preparing, etc. and a final line that says "building 'usdosr_{version number}.tar.gz' "
4. Install the package by typing `R CMD INSTALL 'usdosr_{version number}.tar.gz'`


## Use

An example use case would be to put all of the config files in the working directory and do a parallel loop over them:

```r
library(doParallel)
library(foreach)
library(iterators)
library(doRNG)

library(usdosr)

# Make and register cluster
cl <- makeCluster(3)
registerDoParallel(cl)

# List all config files in working directory
configs <- list.files(path = ".", pattern = "config_")

# run USDOS on those configs
foreach(i = 1:length(configs), .errorhandling = "stop",
        .packages = c("usdosr")) %dopar% {
          
          run_usdos(configs[i])
          
        } 

# Stop the cluster
stopCluster(cl)
        
```


## Documentation

See the project [website](https://webblabb.github.io/usammusdos) for project documentation and manuals.

© 2019 Colorado State University
