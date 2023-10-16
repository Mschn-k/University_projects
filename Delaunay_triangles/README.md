**Delaunay triangles in Digital Terrain Modelling course**

One of my favorite projects has been the assignment for Digital Terrain Modelling course where I had to write a Python script for a program that would generate Delaunay and Voronoi diagrams according to the points inserted by the user. At the start, we were given the interface where a point was added where the user clicked. However, we had to create a code to determine how edges were drawn and changed as new points were added.

Delaunay triangle is formed between 3 points such that neither of the points are inside the circumcircle of another triangle. It is useful because Delaunay triangulations maximise the minimum angle of all the angles of the triangles in the triangulation; they tend to avoid sliver triangles.

![](https://github.com/University_projects/Delaunay_triangles/Delaunay_demo.gif)

In the animation above the outcome can be seen. For each point that the user inserts, three new triangles are created. Meanwhile, if a triangle is tested not to be Delaunay, it is flipped in order to create a triangle with as large angles as possible. The user interface also allows to switch to the view of Voronoi diagram, shown at the end of the clip. A Voronoi vertex is equidistant from 3 data points. 

In the Python code, each triangle is a list of 6 values: 3 indices corner points of the triangles and the 3 other points of neighbouring triangles. The coordinates of the actual points were kept in a separate list, referred to using indices.

The algorithm followed an incremental construction of the Delaunay Triangulation meaning that each point is inserted one at a time in a valid DT and the triangulation is updated. 

**The summary of steps in the algorithm:**

1. Once new point is inserted by the user, the algorithm determines in which existing triangle the point is located. Therefore, not all triangles are analysed through, making the algorithm more efficient.
2. The triangle in which the point lies is removed and three new triangles are created.
3. All of the three new triangles are tested for being true Delaunay triangles - checking if their circumcircle is empty of any other points.
4. If the circumcirle of a newly created triangle is not empty, the triangle is flipped - given four points there is only two ways to triangulate, therefore the second configuration is established.