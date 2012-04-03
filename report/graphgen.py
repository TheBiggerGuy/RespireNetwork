import cairoplot


data = [
    3141,
    1729,
    3248626-3007797-240008-563,
    2507,
    992,
    861,
    221,
    1597,
    6+581
  ]
datax = [
    "11 Dev 2011",
    "18 Dev 2011",
    " 1 Jan 2012",
    " 8 Jan 2012",
    "15 Jan 2012",
    "22 Jan 2012",
    "29 Jan 2012",
    "19 Feb 2012",
    "25 Mar 2012"
  ]
cairoplot.dot_line_plot(
  "images/git_work_time.png",
  data,
  1000, 300,
  x_labels = datax,
  border = 0, axis = True, grid = True,
  x_title = "Date", y_title = "Line of Code changed"
)
