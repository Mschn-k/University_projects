#-- my_code_hw01.py
#-- GEO1015.20019--hw01
#-- Denis Giannelli
#-- 5045304
#-- Katrin Meschin
#-- 5163889


"""
You can add any new functions to this unit, but do not import new units.

You can add new methods to the DT class, but the functions that already exist
must have the same name/structure/input/output.

You need to complete the 2 functions:
  1. insert_one_point(self, x, y)
  2. get_voronoi_edges(self)

The data structure that must be used is:
    pt = [x, y]
    r = [pt1, pt2, pt3, neighbour1, neighbour2, neighbour3]
"""

class DT:
    def __init__(self):
        self.pts = []
        self.trs = []
        #- create infinite triangle
        #- create 3 vertices
        self.pts.append([-10000, -10000])
        self.pts.append([10000, -10000])
        self.pts.append([0, 10000])
        #- create one triangle
        self.trs.append([0, 1, 2, -1, -1, -1])

    def number_of_points(self):
        return len(self.pts)

    def number_of_triangles(self):
        return len(self.trs)

    def get_delaunay_vertices(self):
        return self.pts

    def get_delaunay_edges(self):
        edges = []
        for tr in self.trs:
            a = self.pts[tr[0]]
            b = self.pts[tr[1]]
            c = self.pts[tr[2]]
            edges.append(a)
            edges.append(b)
            edges.append(a)
            edges.append(c)
            edges.append(b)
            edges.append(c)
        return edges

    def get_voronoi_edges(self):

        edges = []
        
        for tr in range(len(self.trs)-1):
            
            tr_object = self.trs[tr]
            centre_self = self.centre_pt(tr)
            a = []
            a.append(tr_object[3])
            a.append(tr_object[4])
            a.append(tr_object[5])

            for i in a:
                if i >=0:
                    k = i
                    n_centre =self.centre_pt(k)
                    edges.append(centre_self)
                    edges.append(n_centre)

        return edges

    def orientation_test(self,edge,pt_index):
        a = edge[0]
        b = edge[1]
        ax = a[0]
        ay = a[1]
        bx = b[0]
        by = b[1]
        
        pt_index = len(self.pts) - 1
        point_object = self.pts[pt_index]

        px = point_object[0]
        py = point_object[1]
        det = (ax*by*1 + ay*1*px + 1*bx*py) - (1*by*px + 1*py*ax + 1*ay*bx )
        return det
    
    def is_point_in_triangle(self, tr_index, pt_index):
        # this is a test function that only works with Brute force.
        # It is not called by any other functions used for the assignment.

        pt_object = self.pts[pt_index]
        tr_object = self.trs[tr_index] # [pt1, pt2, pt3, neighbour1, neighbour2, neighbour3]

        point_a = tr_object[0] # from the triangle object, extract point_a, which is an index.
        point_b = tr_object[1] # from the triangle object, extract point_b, which is an index.
        point_c = tr_object[2] # from the triangle object, extract point_c, which is an index.
        edge_1 = [self.pts[point_b] , self.pts[point_c]] # creates edge_1, which is a list: [ [bx, by], [cx, cy] ]
        edge_2 = [self.pts[point_c] , self.pts[point_a]] # creates edge_1, which is a list: [ [cx, cy], [ax, ay] ]
        edge_3 = [self.pts[point_a] , self.pts[point_b]] # creates edge_1, which is a list: [ [ax, ay], [bx, by] ]

        test1 = self.orientation_test(edge_1, pt_index ) # tests the orientation: output is a number
        test2 = self.orientation_test(edge_2, pt_index ) # tests the orientation: output is a number
        test3 = self.orientation_test(edge_3, pt_index ) # tests the orientation: output is a number

        if test1 <= 0 and test2 <= 0 and test3 <= 0:
            return False
        else:
            return True
    
    def brute_force(self, tr_index, pt_index):
        # this is a test function that only works with Brute force.
        # It is not called by any other functions used for the assignment.
        pt_object = self.pts[pt_index]
        tr_object = self.trs[tr_index]
        for i in tr_object[:3]:
            if self.is_point_in_triangle(tr_object, p) == False:
                tr += 1 # It needs to go to the next triangle in the list.
                self.brute_force(tr, p)
            elif self.is_point_in_triangle(tr_object, p) == True:
                return tr_object

    def walk(self, pt_index):
        pt_index = len(self.pts) - 1
        point_object = self.pts[pt_index]

        tr_index = 0 # I start with the triangle that has index = 0, and then I walk.
        correct_triangle_index = None

        while correct_triangle_index == None:
            tr_object = self.trs[tr_index]
            point_a = tr_object[0] # from the triangle object, extract point_a, which is an index.
            point_b = tr_object[1] # from the triangle object, extract point_b, which is an index.
            point_c = tr_object[2] # from the triangle object, extract point_c, which is an index.
            edge_1 = [self.pts[point_b] , self.pts[point_c]] # creates edge_1, which is a list: [ [bx, by], [cx, cy] ]
            edge_2 = [self.pts[point_c] , self.pts[point_a]] # creates edge_1, which is a list: [ [cx, cy], [ax, ay] ]
            edge_3 = [self.pts[point_a] , self.pts[point_b]] # creates edge_1, which is a list: [ [ax, ay], [bx, by] ]
            # Correspondence:
            # point a - opposite edge_1 (bc) - adjacent triangle tau_a: tr_object[4]
            # point b - opposite edge_2 (ca) - adjacent triangle tau_b: tr_object[5]
            # point c - opposite edge_3 (ab) - adjacent triangle tau_c: tr_object[6]
            # according to the book p. 31
            el_triangle = [point_a, point_b, point_c, edge_1, edge_2, edge_3]
            visited_edges = 0
            
            for i in range(len(el_triangle[:3])): 
                visiting_edge = el_triangle[i+3]
                if self.orientation_test(visiting_edge,pt_index) < 0:
                    tr_index = tr_object[i+3]
                    break
                visited_edges += 1

                # base case
            if visited_edges == 3:
                # all edges of the current triangle have been tested, so this is the triangle.
                correct_triangle_index = tr_index
        return correct_triangle_index
    
    def split_triangle(self, pt_index):
        
        point_object = self.pts[pt_index] # the point object, based on the index in insert_one_point(self, x, y)
        
        selected_triangle_index = self.walk(pt_index) # The index of the triangle object in which P lies into.
        selected_triangle_object = self.trs[selected_triangle_index] # The triangle object in which P lies tinto.
                
        point_a = selected_triangle_object[0] # from the triangle object, extract point_a, which is an index.
        point_b = selected_triangle_object[1] # from the triangle object, extract point_b, which is an index.
        point_c = selected_triangle_object[2] # from the triangle object, extract point_c, which is an index.

        ABP = [ point_a , point_b  , pt_index , 0, 0, 0 ] # all elements are indices, as they should be.
        PBC = [ pt_index, point_b  , point_c  , 0, 0, 0 ] # all elements are indices, as they should be.
        APC = [ point_a , pt_index , point_c  , 0, 0, 0 ] # all elements are indices, as they should be.

        N1 = selected_triangle_object[3] # This assignment should be put here, before the re-assignment of select_triangle_object.
        N2 = selected_triangle_object[4] # This assignment should be put here, before the re-assignment of select_triangle_object.
        N3 = selected_triangle_object[5] # This assignment should be put here, before the re-assignment of select_triangle_object.

        self.trs[selected_triangle_index] = ABP # this code is the way of substitute a triangle with another using their indices.
        self.trs.append(PBC) # this code is the way of appending a triangle in the trs list, based on the triangle object.
        self.trs.append(APC) # this code is the way of appending a triangle in the trs list, based on the triangle object.

        ABP_index = self.trs.index(ABP)
        PBC_index = self.trs.index(PBC)
        APC_index = self.trs.index(APC)

        ABP[3] = PBC_index
        ABP[4] = APC_index
        ABP[5] = selected_triangle_object[5]

        PBC[3] = selected_triangle_object[3]
        PBC[4] = APC_index
        PBC[5] = ABP_index

        APC[3] = PBC_index
        APC[4] = selected_triangle_object[4]
        APC[5] = ABP_index

        N1_object = self.trs[N1]
        N2_object = self.trs[N2]
        N3_object = self.trs[N3]

        N1_points  = N1_object[:3]
        APC_points = APC[:3]

        N2_points  = N2_object[:3]
        PBC_points = PBC[:3]

        N3_points  = N3_object[:3]
        ABP_points = ABP[:3]

        for i in N1_points:
           if i not in PBC_points:
                point_d = i # point_d (index in self.pts), as part of N1 (tr object)
                position_point_d_in_N1 = N1_object[:3].index(point_d)
                PBC_as_neighbour = N1_object[position_point_d_in_N1 + 3]
                PBC_as_neighbour = PBC_index

        for i in N2_points:
           if i not in APC_points:
                point_e = i # point_d (index in self.pts), as part of N1 (tr object)
                position_point_e_in_N2 = N2_object[:3].index(point_e)
                APC_as_neighbour = N2_object[position_point_e_in_N2 + 3]
                APC_as_neighbour = APC_index

        for i in N3_points:
           if i not in ABP_points:
                point_f = i # point_d (index in self.pts), as part of N1 (tr object)
                position_point_f_in_N3 = N3_object[:3].index(point_f)
                ABP_as_neighbour = N3_object[position_point_f_in_N3 + 3]
                ABP_as_neighbour = ABP_index

        
        new_triangles_indices_list = [ABP_index, PBC_index, APC_index]
        return new_triangles_indices_list

    def distance(self, point, other): #finds a distance between two points
        # point : index
        dx = point[0] - other[0]
        dy = point[1] - other[1]

        return math.sqrt(dx**2 + dy**2)

    def centre_pt(self, tr): #returns the centre point of the triangle
        #Takes a triangle as a parameter which is returned from the walk
        #Returns the coordinates of the centre point. X-coordinate of each point would be self.pts[0]
        #tr_pts = self.pts[self.trs[tr]] #gives a list of six x,y coordinates
        tr_object = self.trs[tr]

        a = self.pts[tr_object[0]]
        b = self.pts[tr_object[1]]
        c = self.pts[tr_object[2]]
        #print('a is: ', a, 'b is; ', b, 'c is ', c)

        D = 2.0 * (a[0] * (b[1] - c[1]) + b[0] * (c[1] - a[1]) + c[0] * (a[1] - b[1]))
        ux = ((a[0]**2 + a[1]**2) * (b[1] - c[1]) + (b[0]**2 + b[1]**2) * (c[1] - a[1]) + (c[0]**2 + c[1]**2) * (a[1] - b[1]))/D
        uy = ((a[0]**2 + a[1]**2) * (c[0] - b[0]) + (b[0]**2 + b[1]**2) * (a[0] - c[0]) + (c[0]**2 + c[1]**2) * (b[0] - a[0]))/D
        centr = [ux, uy]

        centr = [ux, uy]
        
        return centr

    def covers(self, tr): 
        # or we check  if the verices of the neighbouring triangles are inside the circumcirle
        tr_object = self.trs[tr]
        
        #If the distance between any of the points of the neighbouring tirangle vertices and the centre of the visiting triangle is bigger than the ditance between the centre point

        centr = self.centre_pt(tr)
        vertex = self.pts[tr_object[0]]
        an = tr_object[0] #returns the point index of each vertex
        bn = tr_object[1]
        cn = tr_object[2]
        print('center is: ', centr, 'vertex is: ', vertex)
        print ('a is ', an)
        neighbours = tr_object[3:] #returns the indeces of the neighbouring triangles
        tr1 = self.pts[neighbours[0]] #returns the x and y of the first neigbour
        print('neighbours are:', neighbours, 'tr1 is: ', tr1)

        for tri in neighbours:
            a = self.trs[neighbours[tri]] # a is a triangle object of each neighbour [pt1, pt2, pt3, adj1, adj2, adj3]
            b = a[:3] # b is a list with the three vertices of the neighbour a [pt1, pt2, pt3]
            c = []
            print ('tirangle is:', tr_object[:3], 'b is: ', b)    
        
            for i in b:
                if i not in tr_object[:3]:
                    c.append(i)
                    point = self.pts[i]
                    print ('point is ', point)
                    dist_n_point_centre = self.distance(point, centr) # distance will be a float number.
                    dist_vertex_centre = self.distance(vertex, centr) # distance will be a float number.
                    print ('distance1 is: ', dist, 'distance 2 is: ', dist2)

                    if dist_n_point_centre < dist_vertex_centre: # If the point is in circle.... not Delaunay.
                        return True
                    else:
                        return False

    def stack(self, tr_index):
        stack = [ ]

        tr_object = self.trs[tr_index]


    def flip(self,tr_index,pt_index):
        
        tr_object = self.trs[tr_index]
        
        point_a = tr_object[0] # from the triangle object, extract point_a, which is an index.
        point_b = tr_object[1] # from the triangle object, extract point_b, which is an index.
        point_c = tr_object[2] # from the triangle object, extract point_c, which is an index.

        pt_object = self.pts[pt_index]

        APC = [ point_a , pt_index  , point_c , 0, 0, 0 ] # all elements are indices, as they should be.
        PBC = [ pt_index , point_b  , point_c , 0, 0, 0 ] # all elements are indices, as they should be.

        N1 = tr_object[3] # This assignment should be put here, before the re-assignment of select_triangle_object.
        N2 = tr_object[4] # This assignment should be put here, before the re-assignment of select_triangle_object.
        N3 = tr_object[5] # This assignment should be put here, before the re-assignment of select_triangle_object.

        self.trs[tr_index] = APC # this code is the way of substitute a triangle with another using their indices.
        self.trs.append(PBC) # this code is the way of appending a triangle in the trs list, based on the triangle object.

        APC_index = self.trs.index(APC)
        PBC_index = self.trs.index(PBC)

        APC[3] = PBC_index
        APC[4] = tr_object[4]
        APC[5] = tr_object[5]

        PBC[3] = tr_object[3]
        PBC[4] = APC_index
        PBC[5] = tr_object[5]

        N1_object = self.trs[N1]
        N2_object = self.trs[N2]
        N3_object = self.trs[N3]

        an = [ APC_index , PBC_index ] 
        return an 

    def insert_one_point(self, x, y):

        self.pts.append((x,y))
        pt_index = len(self.pts) - 1
        print(self.split_triangle(pt_index))
        print( )