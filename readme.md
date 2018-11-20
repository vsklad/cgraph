# CGraph
CGraph is a tool for generating variable incidence graph (VIG) [GraphML](http://graphml.graphdrawing.org) representation of a [CNF](https://en.wikipedia.org/wiki/Conjunctive_normal_form) formula specified in [DIMACS](http://www.satcompetition.org/2009/format-benchmarks2009.html) format.

- Source code is published under [MIT license](https://github.com/vsklad/cgraph/blob/master/LICENSE).
- Source code is available on GitHub: <https://github.com/vsklad/cgraph>.

## Description

Translating [SAT](https://en.wikipedia.org/wiki/Boolean_satisfiability_problem) problems into [CNF](https://en.wikipedia.org/wiki/Conjunctive_normal_form) results in formulas that are usually too large and complex for unaided human perception. Representing a [CNF](https://en.wikipedia.org/wiki/Conjunctive_normal_form) formula as a graph can be an effective means of demonstrating and analysing its structure, particularly if accompanied by graph visualization. 

Back in time, [Carsten Sinz](http://www.carstensinz.de) and his colleagues at the University of Tübingen have done notable work translating SAT instances into graphs and visualizing them. They built [DPvis](http://www.carstensinz.de/software.html) and 3Dvis, tools for the purpose. While DPvis is still available [to download](https://web.archive.org/web/20100309223324/http://www-sr.informatik.uni-tuebingen.de/~sinz/DPvis/DPvis-download.html) from Web Archive, it is outdated. More recently, similar work in the University of Waterloo resulted into [SATGraf](https://bitbucket.org/znewsham/satgraf/src/master/), a tool focused on analysing communities and visualizing evolution of SAT instances while solving.

With respect to CNF formulas, implication graph can be costructed for binary clauses (for example, for a 2-CNF formula) without loss of information. No straightforward translation exists in general case however. At the same time, we are mainly interested to show the complexity and structure of the formula through the connections between CNF variables. DPVis operates with *variable dependency (interaction) graphs* [1, 2, 3]. SATGraf is based on a similar concept of *variable incidence graph* (VIG) [4, 5]. 

Using the above definitions, we build an *undirected* graph as follows:

1. CNF variables are represented as vertices
2. If two variables appear within the same clause, the connection between them is represented as an undirected edge

This representation is "incomplete", that is, the original CNF formula cannot be recovered from the graph.

Optionally, CGraph may output edge weight and cardinality. We define edge cardinality as the number of clauses the particular pair of variables appears in. Weight is calculated as follows:

- edge_clause_weight = 2 / (clause_size * (clause_size - 1))
- edge_weight = sum(edge_clause_weight) for all clauses that contain the variables pair

This definition is consistent with the one used in [5] and ensures the sum of edge weights introduced by a single clause is 1.

In order to derive meaningful structural interpretation, we are particularly interested in CNF variables that correspond to parameters of the underlying SAT problem. This is because a large number of auxiliary variables is introduced into the formula in order to contain its size during translation, whether through applying Tseitin transformation or through crafted CNF representation of the original problem constraints.

Information about the original SAT problem parameters is not present within the CNF formula itself. At the same time, CGraph operates with DIMACS CNF format which can be extended. In particular, CGraph recognizes "named variable" definitions recorded as comments within DIMACS CNF files produced using [our CGen tool](https://github.com/vsklad/cgen). Then, CGraph records underlying SAT parameter name and the associated CNF variable index if defined as atteributes of the graph vertex. For the purpose of visualization, these attributes allow grouping and distinguishing original SAT parameters using color, shape of the vertices and similar means. 

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

CGraph takes the following parameters:

cgraph [-w] input_file_name> [output_file_name]

Where:

- input_file_name - input DIMACS CNF file name
- output_file_name - output Graph ML file name
- w - include edge weight and cardinality

## Acknowledgements & References

Significant proportion of CGraph source code is shared with [CGen](https://github.com/vsklad/cgen).

The concept of visualizing SAT instances using variable incidence graphs is discussed at length in the work referenced below.

1. Carsten Sinz. Visualizing the internal structure of SAT instances (preliminary report). In Proc. of the 7th Intl. Conf. on Theory and Applications of Satisfiability Testing (SAT 2004), Vancouver, Canada, May 2004, url: https://www.satisfiability.org/SAT04/programme/117.pdf
2. Carsten Sinz and Edda-Maria Dieringer.DPvis - a tool to visualize structured SAT instances. In Proc. of the 8th Intl. Conf. on Theory and Applications of Satisfiability Testing (SAT 2005), pages 257-268, St. Andrews, UK, June 2005. Springer-Verlag, url: http://www.carstensinz.de/papers/SAT-2005.pdf
3. Carsten Sinz. Visualizing SAT Instances and Runs of the DPLL Algorithm. J. Automated Reasoning, 39(2):219-243, 2007, url: http://www.carstensinz.de/papers/JAR-2007-Visualization.pdf
4. Newsham, Zack and Lindsay, William and Ganesh, Vijay and Liang, Jia Hui and Fischmeister, Sebastian and Czarnecki, Krzysztof. SATGraf: Visualizing the Evolution of SAT Formula Structure in Solvers, In Proc. of the 18th Intl. Conf. on Theory and Applications of Satisfiability Testing (SAT 2015), pages 62-70, 2015, doi: [10.1007/978-3-319-24318-4_6](http://dx.doi.org/10.1007/978-3-319-24318-4_6)
5. Giráldez Crú, Jesús, Levy Díaz, Jordi, dir., Gonzàlez i Sabaté, Jordi. Beyond the structure of SAT formulas. [Barcelona]: Universitat Autònoma de Barcelona, 2016. 1 recurs electrònic (145 p.). ISBN-9788449064234. Tesi doctoral - Universitat Autònoma de Barcelona. Departament de Ciències de la Computació, 2016 <https://ddd.uab.cat/record/165949>, accessed: 19 November 2018

## About
I developed CGraph part of hobby research into SAT.
If you have any questions about the tool, you can reach me at [vs@sophisticatedways.net](mailto:vs@sophisticatedways.net).
