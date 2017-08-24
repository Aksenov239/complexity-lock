\begin{tikzpicture}
   \begin{groupplot}[
       group style={
           group size= 6 by 5,
       },
       height=5cm,
       width=5cm,
   ]

   \nextgroupplot[title=parallel work: 500,
                  ylabel={Processors: 5},
                  cycle list name=color]
       \addplot table {../../data/d2000/parallel_5_500_simple.dat};\label{plots:500}

   \nextgroupplot[title=parallel work: 1000, cycle list name=color]
       \addplot table {../../data/d2000/parallel_5_1000_simple.dat};\label{plots:500}

   \nextgroupplot[title=parallel work: 5000, cycle list name=color]
       \addplot table {../../data/d2000/parallel_5_5000_simple.dat};\label{plots:500}

   \nextgroupplot[title=parallel work: 10000, cycle list name=color]
       \addplot table {../../data/d2000/parallel_5_10000_simple.dat};\label{plots:500}

   \nextgroupplot[title=parallel work: 50000, cycle list name=color]
       \addplot table {../../data/d2000/parallel_5_50000_simple.dat};\label{plots:500}

   \nextgroupplot[title=parallel work: 100000, cycle list name=color]
       \addplot table {../../data/d2000/parallel_5_100000_simple.dat};\label{plots:500}



   \nextgroupplot[ylabel={Processors: 10},
                  cycle list name=color]
       \addplot table {../../data/d2000/parallel_10_500_simple.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/parallel_10_1000_simple.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/parallel_10_5000_simple.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/parallel_10_10000_simple.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/parallel_10_50000_simple.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/parallel_10_100000_simple.dat};\label{plots:500}



   \nextgroupplot[ylabel={Processors: 20},
                  cycle list name=color]
       \addplot table {../../data/d2000/parallel_20_500_simple.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/parallel_20_1000_simple.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/parallel_20_5000_simple.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/parallel_20_10000_simple.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/parallel_20_50000_simple.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/parallel_20_100000_simple.dat};\label{plots:500}



   \nextgroupplot[ylabel={Processors: 30},
                  cycle list name=color]
       \addplot table {../../data/d2000/parallel_30_500_simple.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/parallel_30_1000_simple.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/parallel_30_5000_simple.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/parallel_30_10000_simple.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/parallel_30_50000_simple.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/parallel_30_100000_simple.dat};\label{plots:500}



   \nextgroupplot[ylabel={Processors: 39},
                  cycle list name=color]
       \addplot table {../../data/d2000/parallel_39_500_simple.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/parallel_39_1000_simple.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/parallel_39_5000_simple.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/parallel_39_10000_simple.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/parallel_39_50000_simple.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/parallel_39_100000_simple.dat};\label{plots:500}
  \end{groupplot}

\end{tikzpicture}