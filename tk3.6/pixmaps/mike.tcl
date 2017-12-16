label .l -bitmap @mike.xpm
puts stderr [pinfo names]
puts stderr [pinfo data @mike.xpm xpm3]
pinfo define @xxx.xpm [pinfo data @mike.xpm xpm3]
puts stderr [pinfo names]
puts stderr [pinfo cache @mike.xpm 1]
puts stderr [pinfo cache]
puts stderr [pinfo cache @xxx.xpm 1]
puts stderr [pinfo cache]
