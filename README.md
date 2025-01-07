// TUDOR ANDREI BICIUSCA 314CA

C program for editing PPM and PGM images.
Support for both plain text and binary files.

The program runs without parameters, once started it will wait for any input.
Allowed inputs are :

LOAD <file> -> takes a *PPM* or *PGM* image and loads it into memory

SELECT <x1> <y1> <x2> <y2> -> [x1,x2) and [y1,y2) are the pixel values that will be affected
    ! 0,0 IS CONSIDERED TO BE TOP-LEFT

SELECT ALL -> selects the whole image for the following commands

ROTATE <angle> -> <angle> has to be a multiple of 90, negative values are allowed

EQUALIZE -> Equalizes the pixel values of the whole image, flattens the histogram
    ! ONLY FOR COLOR IMAGES

CROP -> Cuts the selected portion of the image, and uses it as a new image.

APPLY <effect> -> SHARPEN, BLUR, GAUSSIAN_BLUR, EDGE

SAVE <file> -> after <file> you can add "ascii" if you want the files to be written in *PLAIN* TEXT,
otherwise *BINARY* will be the default

HISTOGRAM <x> <y> -> <x> is maximum number of '*' used to represent the histogram, and <y> is the number of bins used
    ! ONLY FOR BLACK AND WHITE IMAGES

EXIT -> the program won't stop until you input "EXIT"



Code explanation :

[ below we use (...) to symbolise (image_t *photo, char *some_string) ]

*main():*
"image_t photo" is created at the start of the program and will be used for every function.
It holds both the image itself but also every other property of the image that is useful for the operations.

*get_input(image_t *photo):*
is a function that loops itself until the string "EXIT" is read.
It waits for input and then calls the correct function based on what the user wrote.
We have a for loop that iterates through a list of strings until it matches one
then using an array of pointers to functions it calls the right function.
if it reaches the end of the list without matching to any string, then the command what invalid
at the end of the function we call it again to repeat the process

*handle_load(...)*
opens the file in binary because we don't know what type it is
reads the format to know the type of file
sets the pixel_depth to 1/3 (1 for grayscale and 3 for color)
calls one of two functions, described bellow

*text_file_image(...)* for plain text reading
the same as the binary reading mostly
the only differences are the functions we use to read the pixel values
here we read one int at a time instead of the unsigned char

*binary_file_image(...)* for binary reading
frees any memory allocated before in photo
after reading the header, it loops and reads one unsigned char at a time until completed
lastly, closes the file, prints a message to signal sucess and select the full image as a default

*handle_rotate(...)*
checks for any problems such as wrong angle values or no image being loaded
decides which helper function to call

*rotate_full_matrix(...)*
decides if the dimensions need to change based on the angle
allocates a new matrix just in case the dimensions do change
calculates the actual values using one of 3 cases
changes the pointer from the original matrix to the new one, after freeing the old one
makes sure that the selection also changes if needed
    (this allows for multiple rotation calls without the need for the SELECT function)

*rotate_selection(...)*
normalizes angle values to account for negative values
allocates a temporary matrix for the rotation
enters one of 3 (the 4th is a base case) cases based on the angle to calculate the values themselves
copys the values calculated from the temporary matrix to the original one

*handle_equalize(...)*
first checks for the correct format and makes sure everything is fine
uses a matrix holding 256 int values to keep the frequency of every pixel value.
iterates through the matrix and changes every pixel value based on the formula
    ( 255 / AREA * (sum of pixels up to [i]) )
makes sure the values respect the [0,255] range and rounds them as well

*handle_histogram(...)*
classic safety checks
reads the numbers given by the user and makes sure that only 2 were given
    ( by trying to read a dummy value )
uses a matrix holding 256 int values to keep the frequency of every pixel value.
then groups the values based on the number of bins wanted
for everybin we calculate the number of stars printed to the console with the formula
    ( value[current_bin]/value[biggest_bin] * x )

NOTE : HISTOGRAM only prints, doesn't change values
                 only works with PGM
       EQUALIZE only changes, doesn't print values
                only work with PPM

*handle_exit(...)*
frees any memory from photo
calls exit(0); to stop the program

*handle_save(...)*
writes the header to the file
then with a loop the whole image

NOTE : handle_save handles the plain text saving and if it detects the need for binary will
call save_binary_image(...) to handle it

*save_binary_image(...)*
removes junk at end of input (trailling spaces or endlines)
writes the header first then
one unsigned char at a time until the whole image is written.

NOTE : both function close the file at the end to avoid any leaks

*handle_crop(...)*
allocates a new matrix based on the selection
swaps the value from the cropped part of the original to the new matrix
changes the pointer from the original image to the newly allocated one after freeing the original
also changes selection for following operations

*handle_select(...)*
attributes the given coordinates as the selection variables
only done after thorough checking, making sure the format is good and that everything works fine

*handle_apply(...)*
calls get_kernel(...) which is described after
allocates a new matrix "temp" which is at first a copy of the orginal (value wise)
we apply the kernel to all groups of 9 pixels, except the edge values that don't have all neighboors
the new value is put into the temp matrix after both clamping and rounding
we free the original matrix and point to the "temp"

*get_kernel(...)*
holds a 3 dimensional matrix holding 5 2d matrices representing the kernels for 4 effects
SHARPEN BLUR GAUSSIAN_BLUR EDGE and one more for the identity matrix

*initialize_struct(image_t *photo)*
sets all values to 0 or NULL

*free_image(image_t *photo)*
checks if the image is not NULL and then
frees everything


The program keeps asking for inputs until it meets the string "EXIT".
