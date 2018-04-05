// Code for graph
TESTER = document.getElementById('plot');
Plotly.plot( TESTER, [{
   x: [1, 2, 3, 4, 5],
   y: [1, 2, 4, 8, 16] }], {
   paper_bgcolor: 'rgba(0,0,0,0)',
   plot_bgcolor: 'rgba(0,0,0,0)',
   margin: { t: 0 } });
