vertex = []
s = ''

while (True):
  try:
    s = input()
  except:
    break
  try:
    x , y = tuple(map(float, s.strip(', \r\t\n').split(sep = ',')))
    vertex.append([x, y])
  except:
    vertex = []
    print(r'},')
    print(r'{')
  
  l = len(vertex)
  if l >= 3:
    x0 = vertex[-3][0]
    y0 = vertex[-3][1]
    x1 = vertex[-2][0]
    y1 = vertex[-2][1]
    x2 = vertex[-1][0]
    y2 = vertex[-1][1]

    d = lambda x, y: (x**2 + y ** 2)**(1/2)

    dx = x2 - x0
    dy = y2 - y0
    dd = d(dx, dy)
    dx /= dd
    dy /= dd

    d01 = d(x0 - x1, y0 - y1)    
    
    dc = (dx * (x0 - x1) + dy * (y0 - y1))
    nd = (d01 / 2.5) ** 2 / (dc if dc != 0 else 0.00000001)
    if nd > d01 : 
      nd = d01

    trans = lambda x,y: (y*9/16, -x*16/9)
    ax, ay = trans (x1, y1)
    print(ax , ay, sep = 'f,', end = "f,\n")
    ax, ay = trans (dx * nd + x1, dy * nd + y1)
    print(ax, ay, sep = 'f,', end = 'f,\n')
    
    d02 = d(x2 - x1, y2 - y1)
    dc = (dx * (x2 - x1) + dy * (y2 - y1))
    nd = (d02 / 2) ** 2 / (dc if dc != 0 else 0.00000001)
    if nd > d02:
      nd = d02 
    ax, ay = trans (dx * nd + x1, dy * nd + y1)
    print(ax, ay, sep = 'f,', end = 'f,\n')






    d12 = d(x2 - x1, y2 - y1)
    


