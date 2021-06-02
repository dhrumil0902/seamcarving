# seamcarving
Resizes image based on content-awareness using dynamic programming

***What is SeamCarving?***

Seam-carving is a content-aware image resizing technique where the image is reduced in size by one pixel of width (or height) at a time. 
A vertical seam in an image is a path of pixels connected from the top to the bottom with one pixel in each row; a horizontal seam is a path of pixels connected from the left to the right with one pixel in each column. 
Unlike standard content-agnostic resizing techniques (such as cropping and scaling), seam carving preserves the most interest features (aspect ratio, set of objects present, etc.) of the image.

***How the Program Works***

A python program was used to convert a photo into pixels. Then for each pixel, a dual gradient enrgy was calculated using the calc_energy function.

Then,the dynamic_seam function allocates and computes the dunamic array best_arr, which contains the minimum cost of a seam from the top of grad to the point (i,j).

The recover_path function allocates a path through the minimum seam as defined by the best_arr (least amount of energy).

Finally, a numerous amount of seams are removed based on the size of the photo an user wants using remove_seam. The final image is converted back into png/jpeg using python.
