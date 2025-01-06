#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/*
to do
1 ignorare comentarii cand citesti poze |text?|
*/

//  __attribute__((packed))
typedef struct{
    int **pixels;   // 2D array for pixel values
    int width;      // Image width
    int length;     // Image length
    int max_value;  // Maximum pixel value
    int pixel_depth; // Number of channels (1 for grayscale, 3 for RGB)
	int x1,x2,y1,y2; // Values that represent the selected portion of the image
} Image;


void freeImage(Image *photo) {
    if (photo && photo->pixels) {
        for (int i = 0; i < photo->width; i++) {
            free(photo->pixels[i]);
        }
        free(photo->pixels);
        photo->pixels = NULL;
    }
}

void swap(int *x, int *y)
{
	int z = 0;
	z = *x;
	*x = *y;
	*y = z;
}

void sort_int(int *x,int *y)
{
	if(*x > *y)
	{
		swap(x,y);
	}
}

void initialize_struct(Image *photo)
{
	photo->pixels = NULL;
    photo->width = 0;
    photo->length = 0;
    photo->max_value = 0;
   	photo->pixel_depth = 0;
	photo->x1 = 0;
	photo->y1 = 0;
	photo->x2 = 0;
	photo->y2 = 0;
}

void get_kernel(double kernel[3][3], char *filter)
{
    // might need floats instead
    double filters[5][3][3] = {
        {{-1,-1,-1},{-1,8,-1},{-1,-1,-1}},
        {{0,-1,0},{-1,5,-1},{0,-1,0}},
        {{1.0,1.0,1.0},{1.0,1.0,1.0},{1.0,1.0,1.0}},
        {{1.0,2.0,1.0},{2.0,4.0,2.0},{1.0,2.0,1.0}},
        {{0,0,0},{0,1,0},{0,0,0}}
    };
    int selection = 4;
    if(!strcmp(filter,"EDGE\n"))
    {
        selection = 0;
    }
    if(!strcmp(filter,"SHARPEN\n"))
    {
        selection = 1;
    }
    if(!strcmp(filter,"BLUR\n"))
    {
        selection = 2;
    }
    if(!strcmp(filter,"GAUSSIAN_BLUR\n"))
    {
        selection = 3;
    }
    if(selection == 4)
    {
        printf("APPLY parameter invalid\n");
    }
    for(int i = 0;i<3;i++)
    {
        for(int j = 0;j<3;j++)
        {
            kernel[i][j] = filters[selection][i][j];
        }
    }
    filter[strlen(filter)-1] = '\0';
    if(selection == 4)
    {
        return;
    }
    printf("APPLY %s done\n",filter);
}

void handle_apply(Image *photo,char *argument)
{
    if(!photo->length)
    {
        printf("No image loaded\n");
        return;
    }
    if(argument == NULL)
    {
        printf("Invalid command\n");
        return;
    }
    if(photo->pixel_depth == 1)
    {
        printf("Easy, Charlie Chaplin\n");
        return;
    }
    double kernel[3][3];
    int **temp;

    get_kernel(kernel,argument);
    temp = (int **)calloc(photo->width , sizeof(int *));
    if (temp == NULL) {
        perror("Failed to allocate row pointers");
        return;
    }
    int value = 1;
    if(!strcmp(argument,"BLUR"))
    {
        value = 9;
    }
    if(!strcmp(argument,"GAUSSIAN_BLUR"))
    {
        value = 16;
    }
    // Allocate memory for each row
    for (int i = 0; i < photo->width; i++) {
        (temp)[i] = (int *)calloc(photo->length * photo->pixel_depth , sizeof(int));
        // replace this with the freeImage() if time allows
        if ((temp)[i] == NULL) {
            perror("Failed to allocate row");
            // Free any previously allocated rows
            for (int j = 0; j < i; j++) {
                free((temp)[j]);
            }
            free(temp);
            //*temp = NULL;
            return;
        }
    }
    double sum = 0.0;
    for(int i = 0 ;i<photo->width;i++)
    {
        for(int j = 0;j<photo->length * photo->pixel_depth;j++)
        {
            temp[i][j] = photo->pixels[i][j];
        }
    }
    for (int i = photo->y1; i < photo->y2; i++) {
        for (int j = photo->x1 * photo->pixel_depth; j < photo->x2 * photo->pixel_depth; j++) {
            sum = 0;
            if(i == 0 || j < 3 || j >= (photo->length * photo->pixel_depth - 3) || i == photo->width - 1)
                temp[i][j] = photo->pixels[i][j];
            else {
                sum += photo->pixels[i-1][j-3] * kernel[0][0];
                sum += photo->pixels[i-1][j] * kernel[0][1];
                sum += photo->pixels[i-1][j+3] * kernel[0][2];
                sum += photo->pixels[i][j-3] * kernel[1][0];
                sum += photo->pixels[i][j] * kernel[1][1];
                sum += photo->pixels[i][j+3] * kernel[1][2];
                sum += photo->pixels[i+1][j-3] * kernel[2][0];
                sum += photo->pixels[i+1][j] * kernel[2][1];
                sum += photo->pixels[i+1][j+3] * kernel[2][2];
                sum /= value;
                if(sum < 0)
                    sum = 0;
                if(sum > 255)
                    sum = 255;
                temp[i][j] = (int)(round(sum));
                //temp[i][j] = (int)(sum);
            }
        }
    }
     // Free the original matrix
    freeImage(photo);
    // Update the photo structure
    photo->pixels = temp;
}

void handle_select(Image *photo,char *argument)
{
	int x1,x2,y1,y2;
    if(!photo->length)
    {
        printf("No image loaded\n");
        return;
    }
	if(!strcmp(argument,"ALL\n") || !strcmp(argument,"ALL \n"))
	{
		photo->x1 = 0;
		photo->x2 = photo->length;
		photo->y1 = 0;
		photo->y2 = photo->width;
		printf(("Selected ALL\n"));
		return;
	}
	if (sscanf(argument, "%d %d %d %d", &x1, &y1, &x2, &y2) != 4) {
        printf("Invalid command\n");
		return;
    }
	sort_int(&x1,&x2);
	sort_int(&y1,&y2);
	// checks if out of bounds (adica nush daca trebuie pus -1 la fiecare coordonata sau nu depinde daca poti sa iei marginea sau nu)
	if(x1 == x2 || y1 == y2 || x1 < 0 || x2 < 0 || y1 < 0 || y2 < 0 || x1 > photo->length || x2 > photo->length || y1 > photo->width || y2 > photo->width)
	{
        // photo->x1 = 0;
		// photo->x2 = photo->length;
		// photo->y1 = 0;
		// photo->y2 = photo->width;
		printf("Invalid set of coordinates\n");
		return;
	}
	photo->x1 = x1;
	photo->y1 = y1;
	photo->x2 = x2;
	photo->y2 = y2;
	printf("Selected %d %d %d %d\n",photo->x1,photo->y1,photo->x2,photo->y2);
}

// am o presimitre ca e un mem leak aici ca nu omor memoria care era inainte de crop
// trebuie verificat ce considera ei ca fiind out of bounds pls ( defapt asta ar fi la select dar whatever)
void handle_crop(Image *photo, char *argument) {

    if(argument != NULL)
    {
        return;
    }

    if (!photo->length || !photo->width) {
        printf("No image loaded\n");
        return;
    }

    // Calculate the dimensions of the cropped area
    int new_width = photo->y2 - photo->y1;
    int new_length = photo->x2 - photo->x1;

    // Allocate memory for the new cropped matrix
    int **new_pixels = (int **)calloc(new_width, sizeof(int *));
    if (!new_pixels) {
        perror("Failed to allocate memory for cropped matrix");
        return;
    }

    for (int i = 0; i < new_width; i++) {
        new_pixels[i] = (int *)calloc(new_length * photo->pixel_depth, sizeof(int));
        if (!new_pixels[i]) {
            perror("Failed to allocate memory for cropped matrix row");
            // Free any previously allocated rows
            for (int j = 0; j < i; j++) {
                free(new_pixels[j]);
            }
            free(new_pixels);
            return;
        }
    }

    // Copy the selected portion of the original matrix to the new matrix
    int a = 0, b = 0;
    for (int i = photo->y1; i < photo->y2; i++) {
        for (int j = photo->x1 * photo->pixel_depth; j < photo->x2 * photo->pixel_depth; j++) {
            new_pixels[a][b++] = photo->pixels[i][j];
        }
        a++;
        b = 0;
    }

    // Free the original matrix
    freeImage(photo);
    // Update the photo structure
    photo->pixels = new_pixels;
    photo->length = new_length;
    photo->width = new_width;
    photo->x1 = 0;
    photo->y1 = 0;
    photo->x2 = new_length;
    photo->y2 = new_width;
    printf("Image cropped\n");
}

void save_binary_image(Image *photo, char *file_path) {
    if (!photo->length) {
        printf("No image loaded\n");
        return;
    }

    while(file_path[strlen(file_path)-1] == '\n' || file_path[strlen(file_path)-1] == ' ')
    {
        file_path[strlen(file_path)-1] = '\0';
    }

    FILE *file = fopen(file_path, "wb"); // Open the file in binary mode

    if (!file) {
        perror("Failed to open file");
        return;
    }
    // Determine the format (P5 for grayscale, P6 for RGB)
    char *format = NULL;
    if (photo->pixel_depth == 1) { // Assuming 'g' indicates grayscale
        format = "P5";
    } else
        format = "P6";

    // Write the header
    fprintf(file, "%s\n%d %d\n%d\n", format, photo->length, photo->width, photo->max_value);

    // Write pixel data
    // for (int i = 0; i < photo->width; i++) {
    //     fwrite(photo->pixels[i], sizeof(unsigned char), photo->length, file);
    // }
    for(int i = 0;i<photo->width;i++)
    {
        for(int j = 0;j<photo->length * photo->pixel_depth;j++)
        {
            unsigned char pixel_value = photo->pixels[i][j];
            fwrite(&pixel_value, sizeof(unsigned char),1,file);
        }
    }

    fclose(file);
    printf("Saved %s\n",file_path);
}

void handle_save(Image *photo,char *argument)
{
    // For ascii the implementation is here
    // For binary in a seperate function we call here
    if(!photo->length)
    {
        printf("No image loaded\n");
        return;
    }
	if(argument[strlen(argument)-3] == 'i')
    {
        char *file_path = strtok(argument," ");
        FILE *file = fopen(file_path,"w");
        if (!file) {
            perror("Failed to open file");
            return;
        }
        // P2
        if(photo->pixel_depth == 1) {
            fprintf(file,"P2");
        } else {
            fprintf(file,"P3");
        }
        fprintf(file,"\n%d %d\n%d\n",photo->length,photo->width,photo->max_value);
        for(int i = 0;i<photo->width;i++)
        {
            for(int j = 0; j < photo->length * photo->pixel_depth; j++)
            {
                fprintf(file,"%d ",photo->pixels[i][j]);
            }
            fprintf(file,"\n");
        }
        printf("Saved %s\n",file_path);
        fclose(file);
    }
    else
    {
        save_binary_image(photo,argument);
    }
}
void handle_exit(Image *photo,char *argument)
{
    if(argument)
    {
        return;
    }
    if(!photo->length)
    {
        printf("No image loaded\n");
    }
    else
    {
        freeImage(photo);
    }
	exit(0);
}
void handle_print(Image *photo,char *argument)
{
    if(argument)
    {
        return;
    }
	if(!photo->length)
	{
		printf("NO IMAGE LOADED\n");
		return;
	}
	// for(int i = 0;i < photo->width; i++)
	// {
	// 	for(int j =0; j < photo->length * photo->pixel_depth; j++)
	// 	{
	// 		printf("%d ",photo->pixels[i][j]);
	// 	}
	// 	printf("\n");
	// }
    printf("%d %d %d %d\n",photo->x1,photo->x2,photo->y1,photo->y2);
}
void handle_histogram(Image *photo,char *argument)
{
	if(!photo->length)
    {
        printf("No image loaded\n");
        return;
    }
    if(argument == NULL)
    {
        printf("Invalid command\n");
        return;
    }
    int x,y;
    int error;
    float cy;
    if (sscanf(argument, "%d %d %d", &x, &y, &error) != 2) {
        printf("Invalid command\n");
		return;
    }
    cy = y;
    if(y == 0 || y == 1)
    {
        printf("Invalid set of parameters\n");
        return;
    }
    while(cy != 1)
    {
        cy/=2.0;
        if(cy != (int)(cy))
        {
            printf("Invalid set of parameters\n");
		    return;
        }
    }
    if(photo->pixel_depth == 3)
    {
        printf("Black and white image needed\n");
        return;
    }
    int freq[256] = {0};
    int values[256] = {0};
    int group_size = 256/y;
    int sum = 0,max_sum = 0,c_group_size = group_size;int it = 0;
    for(int i  =0 ;i<photo->width;i++)
    {
        for(int j=0;j<photo->length;j++)
        {
            freq[photo->pixels[i][j]]++;
        }
    }
    for(int i = 0;i<256;i++)
    {
        sum = 0;
        while(c_group_size)
        {
            sum+=freq[i++];
            c_group_size--;
        }
        values[it++] = sum;
        if(sum > max_sum)
        {
            max_sum = sum;
        }
        c_group_size = group_size;
        i--;
    }
    for(int i = 0;i<it;i++)
    {
        double stars_f = 1.0*values[i]/max_sum * x;
        int stars = (int)(stars_f);
        printf("%d\t|\t",stars);
        while(stars)
        {
            printf("*");
            stars--;
        }
        printf("\n");
    }
}

void handle_equalize(Image *photo,char *argument)
{
    if(argument)
    {
        return;
    }
	if(!photo->length)
    {
        printf("No image loaded\n");
        return;
    }
    if(photo->pixel_depth == 3)
    {
        printf("Black and white image needed\n");
        return;
    }
    int freq[256] = {0};
    for(int i  =0 ;i<photo->width;i++)
    {
        for(int j=0;j<photo->length;j++)
        {
            freq[photo->pixels[i][j]]++;
        }
    }
    for(int i = 255;i>=0;i--)
    {
        for(int j = 0;j<i;j++)
        {
            freq[i]+=freq[j];
        }
    }
    for(int i = 0;i<photo->width;i++)
    {
        for(int j = 0; j < photo->length; j++)
        {
            double new_value = 0;
            new_value = 255.0/photo->width/photo->length*freq[photo->pixels[i][j]];
            if(new_value > 255)
            {
                new_value = 255;
            }
            photo->pixels[i][j] = (int)(round(new_value));
        }
    }
    printf("Equalize done\n");
}

void rotate_full_matrix(Image *photo, int degrees) {
    // Normalize degrees to [0, 360)
    degrees = (degrees % 360 + 360) % 360;

    int new_width = (degrees == 90 || degrees == 270) ? photo->length : photo->width;
    int new_length = (degrees == 90 || degrees == 270) ? photo->width : photo->length;
    (degrees == 90 || degrees == 270) ? swap(&photo->x2,&photo->y2):1;
    // Allocate a new matrix for the rotated image
    int **rotated_pixels = (int **)malloc(new_width * sizeof(int *));
    for (int i = 0; i < new_width; i++) {
        rotated_pixels[i] = (int *)malloc(new_length * photo->pixel_depth * sizeof(int)); // 3 channels per pixel
    }
    if (degrees == 90) {
        for (int i = 0; i < photo->width; i++) {       // Rows
            for (int j = 0; j < photo->length; j++) {  // Columns
                for (int k = 0; k < photo->pixel_depth; k++) {
                    rotated_pixels[j][(photo->width - 1 - i) * photo->pixel_depth + k] = photo->pixels[i][j * photo->pixel_depth + k];
                }
            }
        }
    } else if (degrees == 180) {
        for (int i = 0; i < photo->width; i++) {       // Rows
            for (int j = 0; j < photo->length; j++) {  // Columns
                for (int k = 0; k < photo->pixel_depth; k++) {
                    rotated_pixels[photo->width - 1 - i][(photo->length - 1 - j) * photo->pixel_depth + k] = photo->pixels[i][j * photo->pixel_depth + k];
                }
            }
        }
    } else if (degrees == 270) {
        for (int i = 0; i < photo->width; i++) {       // Rows
            for (int j = 0; j < photo->length; j++) {  // Columns
                for (int k = 0; k < photo->pixel_depth; k++) {
                    rotated_pixels[photo->length - 1 - j][i * photo->pixel_depth + k] = photo->pixels[i][j * photo->pixel_depth + k];
                }
            }
        }
    }

    // Free the old pixel matrix and assign the new one
    for (int i = 0; i < photo->width; i++) {
        free(photo->pixels[i]);
    }
    free(photo->pixels);

    // Update photo properties
    photo->pixels = rotated_pixels;
    photo->width = new_width;   // Update the number of rows
    photo->length = new_length; // Update the number of columns
}

void rotate_selection(Image *photo, int degrees) {
    int cols = photo->x2 - photo->x1; // Number of columns (x-axis range)
    int rows = photo->y2 - photo->y1; // Number of rows (y-axis range)
    // Normalize negative rotations
    if (degrees == -90) {
        degrees = 270;
    } else if (degrees == -270) {
        degrees = 90;
    } else if (degrees == -180) {
        degrees = 180;
    }

    // Temporary matrix for the rotated submatrix
    int **temp = (int **)malloc(cols * sizeof(int *));
    for (int i = 0; i < cols; i++) {
        temp[i] = (int *)malloc(rows * photo->pixel_depth * sizeof(int));
    }

    if (degrees == 90) {
        for (int i = 0; i < rows; i++) { // Rows
            for (int j = 0; j < cols; j++) { // Columns
                for (int k = 0; k < photo->pixel_depth; k++) {
                    temp[j][(rows - i - 1) * photo->pixel_depth + k] =
                        photo->pixels[photo->y1 + i][(photo->x1 + j) * photo->pixel_depth + k];
                }
            }
        }
    } else if (degrees == 180) {
        for (int i = 0; i < rows; i++) { // Rows
            for (int j = 0; j < cols; j++) { // Columns
                for (int k = 0; k < photo->pixel_depth; k++) {
                    temp[rows - i - 1][(cols - j - 1) * photo->pixel_depth + k] =
                        photo->pixels[photo->y1 + i][(photo->x1 + j) * photo->pixel_depth + k];
                }
            }
        }
    } else if (degrees == 270) {
        for (int i = 0; i < rows; i++) { // Rows
            for (int j = 0; j < cols; j++) { // Columns
                for (int k = 0; k < photo->pixel_depth; k++) {
                    temp[cols - j - 1][i * photo->pixel_depth + k] =
                        photo->pixels[photo->y1 + i][(photo->x1 + j) * photo->pixel_depth + k];
                }
            }
        }
    } else {
        printf("WRONG DEGREE\n");
    }

    // Copy rotated submatrix back to the original matrix
    for (int i = 0; i < rows; i++) { // Rows
        for (int j = 0; j < cols; j++) { // Columns
            for (int k = 0; k < photo->pixel_depth; k++) {
                photo->pixels[photo->y1 + i][(photo->x1 + j) * photo->pixel_depth + k] =
                    temp[i][j * photo->pixel_depth + k];
            }
        }
    }

    // Free the temporary matrix
    for (int i = 0; i < cols; i++) {
        free(temp[i]);
    }
    free(temp);
}

void handle_rotate(Image *photo, char *argument) {
    // Parse the degree of rotation from the argument
    int degrees = atoi(argument);
    // Normalize degrees to the range [0, 360]
    if(!photo->length)
    {
        printf("No image loaded\n");
        return;
    }
    if(degrees%90)
    {
        printf("Unsupported rotation angle\n");
        return;
    }
	if(!(degrees%360))
	{
        printf("Rotated %d\n", degrees);
		return;
	}
    if(!photo->x1 && !photo->y1 && photo->x2 == photo->length && photo->y2 == photo->width && photo->width != photo->length)
	{
		rotate_full_matrix(photo,degrees);
	}
	else
	{
        if(photo->x2 - photo->x1 != photo->y2 - photo->y1)
        {
            printf("Invalid set of coordinates");
            return;
        }
		rotate_selection(photo,degrees);
	}
	printf("Rotated %d\n", degrees);
    // Check pixel depth and call the appropriate function
	// verifica instant daca selectia e ALL, daca nu baga ce e mai sus cu schimbarea de unghiuri in fucntia respectiva ca sa poti sa printezi ce trebuie
}

void text_file_image(Image *photo, char * file_name)
{
	// this function will take the current loaded file and repplace it with the new one
	freeImage(photo);
	FILE *file = fopen(file_name,"r");
	fseek(file,2,SEEK_SET);
	fscanf(file," %d %d %d",&photo->length,&photo->width,&photo->max_value);

	// Allocate memory for the row pointers
    photo->pixels = (int **)calloc(photo->width , sizeof(int *));
    if (photo->pixels == NULL) {
        perror("Failed to allocate row pointers");
		fclose(file);
        return;
    }

    // Allocate memory for each row
    for (int i = 0; i < photo->width; i++) {
        (photo->pixels)[i] = (int *)calloc(photo->length * photo->pixel_depth , sizeof(int));
        if ((photo->pixels)[i] == NULL) {
            perror("Failed to allocate row");
            // Free any previously allocated rows
            for (int j = 0; j < i; j++) {
                free((photo->pixels)[j]);
            }
            free(photo->pixels);
            *photo->pixels = NULL;
			fclose(file);
            return;
        }
    }
	for (int i = 0; i < photo->width; i++) {
        for (int j = 0; j < photo->length * photo->pixel_depth; j++) {
            fscanf(file, "%d", &photo->pixels[i][j]);
        }
    }
	printf("Loaded %s\n",file_name);
    photo->x1 = 0;
    photo->x2 = photo->length;
    photo->y1 = 0;
    photo->y2 = photo->width;
	fclose(file);
}

void binary_file_image(Image *photo, char * file_name)
{
	//free_image(photo); // trebie rezolvat pt mem leak dar momentan whatever
	freeImage(photo);
    FILE *file = fopen(file_name, "rb");

    if (!file) {
        perror("Failed to open file");
        return;
    }
    fseek(file,2,SEEK_SET);
    // Read image dimensions and properties
    fscanf(file," %d %d %d\n",&photo->length,&photo->width,&photo->max_value);
    // Allocate memory for 2D pixel array
    photo->pixels = (int **)calloc(photo->width, sizeof(int *));

    if (!photo->pixels) {
        perror("Failed to allocate memory for pixel rows");
        free(photo);
        fclose(file);
        return;
    }

    for (int i = 0; i < photo->width; i++) {
        photo->pixels[i] = (int *)calloc(photo->length * photo->pixel_depth,sizeof(int));
        if (!photo->pixels[i]) {
            perror("Failed to allocate memory for pixel columns");
            for (int j = 0; j < i; j++) {
                free(photo->pixels[j]);
            }
            free(photo->pixels);
            free(photo);
            fclose(file);
            return;
        }
    }

    // Read pixel data into the 2D array
    for (int i = 0; i < photo->width; i++) {
        for(int j = 0;j < photo->length * photo->pixel_depth;j++)
        {
            unsigned char pixel_value;
            fread(&pixel_value, sizeof(unsigned char),1, file);
            photo->pixels[i][j] = pixel_value;
        }
    }
    fclose(file);
	printf("Loaded %s\n",file_name);
    photo->x1 = 0;
    photo->x2 = photo->length;
    photo->y1 = 0;
    photo->y2 = photo->width;
    return;
}

void handle_load(Image *photo, char *file_name)
{
	char format[3] = {0};
	if(file_name == NULL) {
		printf("Failed to load %s\n", file_name);
		return;
	}
	int l = strlen(file_name);
	file_name[l-1] = '\0';
	FILE *file = fopen(file_name,"rb");
	if (!file) {
        printf("Failed to load %s\n", file_name);
        freeImage(photo);
        initialize_struct(photo);
        return; // Failed to load
    }
	fread(format,sizeof(char),2,file);
	fclose(file);
	if(!strcmp(format,"P2"))
	{
		photo->pixel_depth = 1;
		text_file_image(photo,file_name);
	}
	if(!strcmp(format,"P3"))
	{
		photo->pixel_depth = 3;
		text_file_image(photo,file_name);
	}
	if(!strcmp(format,"P5"))
	{
		photo->pixel_depth = 1;
		binary_file_image(photo,file_name);
	}
	if(!strcmp(format,"P6"))
	{
		photo->pixel_depth = 3;
		binary_file_image(photo,file_name);
	}
	return;
}

void get_input(Image *photo)
{
	char input[101] = {'\0'};
	char *mode;
	char *argument;
	const char *commands[] = {"APPLY", "LOAD", "CROP", "SAVE", "EXIT", "SELECT",
							  "HISTOGRAM", "EQUALIZE", "ROTATE", "PRINT"};
	void (*handlers[])() = {handle_apply,handle_load, handle_crop, handle_save, handle_exit,
                            handle_select, handle_histogram, handle_equalize, handle_rotate, handle_print};
	int list_length = sizeof(commands) / sizeof(commands[0]);

	if (fgets(input, sizeof(input), stdin) == NULL) {
		printf("EROARE LA CITIRE LINII");
		return;
    }

	mode = strtok(input, " ");
	argument = strtok(NULL,"");
    if(argument == NULL && strcmp(mode,"EXIT"))
    {
        mode[strlen(mode)-1] = '\0';
    }
	for(int i = 0; i < list_length; i++)
	{
		if(!strcmp(mode,commands[i]))
		{
			handlers[i](photo,argument);
			break;
		}
		if(i == list_length-1)
		{
			printf("Invalid command\n");
		}
	}
	get_input(photo);
}

int main()
{
	Image photo;
	initialize_struct(&photo);
	get_input(&photo);
	freeImage(&photo);
	//initialize_struct(&photo);
	return 0;
}
