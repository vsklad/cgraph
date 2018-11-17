# CGraph
CGraph is a tool for generating [GraphML](http://graphml.graphdrawing.org) graph representation of a [CNF](https://en.wikipedia.org/wiki/Conjunctive_normal_form) formula specified in [DIMACS](http://www.satcompetition.org/2009/format-benchmarks2009.html) format.

- Source code is published under [MIT license](https://github.com/vsklad/cgraph/blob/master/LICENSE).
- Source code is available on GitHub: <https://github.com/vsklad/cgraph>.

## Description

Translating [SAT](https://en.wikipedia.org/wiki/Boolean_satisfiability_problem) problems into [CNF](https://en.wikipedia.org/wiki/Conjunctive_normal_form) results in formulas that are usually too large and complex for unaided human perception. Representing a [CNF](https://en.wikipedia.org/wiki/Conjunctive_normal_form) formula as a graph can be an effective means of demonstrating and analysing its structure, particularly if accompanied by graph visualization. 

[Carsten Sinz](http://www.carstensinz.de) and his colleagues at the University of Tübingen, Germany, have done notable work back at the time translating SAT instances into graphs and visualising them. They built [DPVis](http://www.carstensinz.de/software.html), a tool for the purpose which is although still available [to download](https://web.archive.org/web/20100309223324/http://www-sr.informatik.uni-tuebingen.de/~sinz/DPvis/DPvis-download.html) from Web Archive, is outdated.

With respect to CNF formulas, implication graph can be costructed for binary clauses (for example, for a 2-CNF formula) without loss of information. No straightforward translation exists in general case however.

At the same time, we are mainly interested to show the complexity and structure of the formula through the connections between CNF variables. DPVis operates with *variable dependency (interaction) graphs*, the concept we use in this case as well. In particular, we build an *undirected* graph as follows:

1. CNF variables are represented as vertices
2. If two variables appear within the same clause, the connection between them is represented as an undirected edge

This representation is "incomplete", that is, the original CNF formula cannot be recovered from the graph.

In order to derive meaningful structural interpretation, we are particularly interested in CNF variables that correspond to parameters of the underlying SAT problem. This is because a large number of auxiliary variables is introduced into the formula in order to contain its size during translation, whether through applying Tseitin transformation or through crafted CNF representation of the original problem constraints.

 Information about the original SAT problem parameters is not present within the CNF formula itself. At the same time, CGraph operates with CNF DIMACS format which can be extended. In particular, CGraph recognizes "named variable" definitions recorded as comments within DIMACS files when CNF file is produced using [our CGen tool](https://github.com/vsklad/cgen). Then, CGraph records underlying SAT parameter name and the associated CNF variable index if defined as atteributes of the graph node/vertice. For the purpose of visualisation, these attributes allow grouping and distinguishing original SAT parameters using color, shape of the nodes and similar means. 
 
We chose [GraphML](http://graphml.graphdrawing.org) for storing the graph as it is a well known extensible format supported by many graph visualization tools including [Gephi](https://gephi.org) and [Cytoscape](https://cytoscape.org).

## Usage

### Build
Source code is hosted at GitHub with CMake (3.1) build file included. 

To obtain and build:

    git clone https://github.com/vsklad/cgraph
    cd cgraph
    cmake .
    make
    
CGraph has no external dependencies other than [C++ STL](https://en.wikipedia.org/wiki/Standard_Template_Library). [C++ 11](https://en.wikipedia.org/wiki/C%2B%2B11) is a requirement.

### Run

CGraph takes two parameters, the first one is a CNF DIMACS file, the second is the output GraphML file name.

## Acknowledgements & References

Significant proportion of CGraph source code is shared with [CGen](https://github.com/vsklad/cgen).

The concept of visualising SAT instances using variable dependency (interaction) graphs is discussed at length in the work referenced below.

1. Carsten Sinz, 2004, Visualizing the Internal Structure of SAT Instances (Preliminary Report), International Conference on Theory and Applications of Satisfiability Testing, url: https://www.satisfiability.org/SAT04/programme/117.pdf
2. Carsten Sinz and Edda-Maria Dieringer, 2005, Visualizing the Internal Structure of SAT Instances (Preliminary Report), International Conference on Theory and Applications of Satisfiability Testing, url: http://www.carstensinz.de/papers/SAT-2005.pdf

## About
I developed CGraph part of hobby research into SAT.
If you have any questions about the tool, you can reach me at [vs@sophisticatedways.net](mailto:vs@sophisticatedways.net).
