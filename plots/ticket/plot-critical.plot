\begin{tikzpicture}
   \begin{groupplot}[
       group style={
           group size= 6 by 5,
       },
       height=5cm,
       width=5cm,
   ]

   \nextgroupplot[title=Critical work: 500,
                  ylabel={Processors: 5},
                  cycle list name=color]
       \addplot table {../../data/d2000/critical_5_500_ticket.dat};\label{plots:500}

   \nextgroupplot[title=Critical work: 1000, cycle list name=color]
       \addplot table {../../data/d2000/critical_5_1000_ticket.dat};\label{plots:500}

   \nextgroupplot[title=Critical work: 5000, cycle list name=color]
       \addplot table {../../data/d2000/critical_5_5000_ticket.dat};\label{plots:500}

   \nextgroupplot[title=Critical work: 10000, cycle list name=color]
       \addplot table {../../data/d2000/critical_5_10000_ticket.dat};\label{plots:500}

   \nextgroupplot[title=Critical work: 50000, cycle list name=color]
       \addplot table {../../data/d2000/critical_5_50000_ticket.dat};\label{plots:500}

   \nextgroupplot[title=Critical work: 100000, cycle list name=color]
       \addplot table {../../data/d2000/critical_5_100000_ticket.dat};\label{plots:500}



   \nextgroupplot[ylabel={Processors: 10},
                  cycle list name=color]
       \addplot table {../../data/d2000/critical_10_500_ticket.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/critical_10_1000_ticket.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/critical_10_5000_ticket.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/critical_10_10000_ticket.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/critical_10_50000_ticket.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/critical_10_100000_ticket.dat};\label{plots:500}



   \nextgroupplot[ylabel={Processors: 20},
                  cycle list name=color]
       \addplot table {../../data/d2000/critical_20_500_ticket.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/critical_20_1000_ticket.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/critical_20_5000_ticket.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/critical_20_10000_ticket.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/critical_20_50000_ticket.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/critical_20_100000_ticket.dat};\label{plots:500}



   \nextgroupplot[ylabel={Processors: 30},
                  cycle list name=color]
       \addplot table {../../data/d2000/critical_30_500_ticket.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/critical_30_1000_ticket.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/critical_30_5000_ticket.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/critical_30_10000_ticket.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/critical_30_50000_ticket.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/critical_30_100000_ticket.dat};\label{plots:500}



   \nextgroupplot[ylabel={Processors: 39},
                  cycle list name=color]
       \addplot table {../../data/d2000/critical_39_500_ticket.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/critical_39_1000_ticket.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/critical_39_5000_ticket.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/critical_39_10000_ticket.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/critical_39_50000_ticket.dat};\label{plots:500}

   \nextgroupplot[cycle list name=color]
       \addplot table {../../data/d2000/critical_39_100000_ticket.dat};\label{plots:500}
  \end{groupplot}

%  \path (top|-current bounding box.north)--
%        coordinate(legendpos)
%       (bot|-current bounding box.north);
\end{tikzpicture}
