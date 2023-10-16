#-- interface.py
#-- GEO1015.20019--hw01
#-- Hugo Ledoux <h.ledoux@tudelft.nl>
#-- 2019-09-30

#------------------------------------------------------------------------------
# DO NOT MODIFY THIS FILE!!!
#------------------------------------------------------------------------------


from tkinter import *
from my_code_hw01 import DT

class MyInterface(Tk):
    def __init__(self):
        Tk.__init__(self)
        self.size = 500
        self.title("GEO1015 hw01 -- my DT/VD")
        self.resizable(0,0)
        self.bind('q', self.exit)
        self.bind('r', self.reset)
        self.bind('d', self.toggle_drawing_dtvd)
        self.drawdt = True
        self.canvas = Canvas(self, bg="white", width=self.size, height=self.size)
        self.canvas.pack()
        self.set_display()
        self.dt = DT()
        self.draw()
        self.output_info()
        
    def set_display(self):
        self.bind("<Motion>", self.display_coords_text)
        self.bind("<ButtonRelease>", self.mouse_click)
        self.coordstext = self.canvas.create_text(self.size, self.size, fill='grey', anchor='se', text='')

    def toggle_drawing_dtvd(self, event):
        if self.drawdt == True:
            self.drawdt = False
        else:
            self.drawdt = True
        self.draw()

    def mouse_click(self, event):
        x = event.x
        y = self.size - event.y
        print("Insert new point: ({}, {})".format(x,y))
        self.dt.insert_one_point(x, y)
        self.draw()

    def output_info(self):
        print("===== USAGE =====")
        print("keyboard 'd' to toggle between DT and VD.")
        print("keyboard 'q' to quit the program.")
        print("keyboard 'r' to reset the DT/VD to an empty one.")
        print("=================")

    def draw_voronoi(self):
        edges = self.dt.get_voronoi_edges()
        for i in range(0, len(edges), 2):
            self.draw_edge(edges[i][0], edges[i][1], edges[i+1][0], edges[i+1][1], 'red')
        
    def draw_delaunay(self):
        edges = self.dt.get_delaunay_edges()
        for i in range(0, len(edges), 2):
            self.draw_edge(edges[i][0], edges[i][1], edges[i+1][0], edges[i+1][1], 'black')
        
    def display_coords_text(self, event):
        s = "(%d,%d)" % (event.x, self.size - event.y)
        self.canvas.itemconfig(self.coordstext, text=s)

    def draw_point(self, x, y, colour):
        radius = 3
        self.canvas.create_oval(x - radius, self.size - y - radius, x + radius, self.size - y + radius, fill=colour)

    def draw_edge(self, x1, y1, x2, y2, colour):
        # colour = "black"
        self.canvas.create_line(x1, self.size - y1, x2, self.size - y2, fill=colour)

    def draw(self):
        self.canvas.delete("all")
        self.set_display()
        pts = self.dt.get_delaunay_vertices()
        for pt in pts:
            self.draw_point(pt[0], pt[1], 'blue')
        if self.drawdt == True:
            self.draw_delaunay()
        else:
            self.draw_voronoi()
        self.canvas.create_text(5, self.size, 
            anchor='sw', 
            fill='grey',
            text='pts={} trs={}'.format(self.dt.number_of_points(), self.dt.number_of_triangles()))
    
    def reset(self, event):
        print ("Reset DT to empty one")
        self.dt = DT()
        self.draw()
    
    def exit(self, event):
        print ("bye bye.")
        self.destroy()


