#include "pdfio.h"
#include <cairo/cairo.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

// -- Graphics State Structure --
typedef struct
{
  // ToDo -- Add more state variable here later (fonts, etc)
  double fill_rgb[3]; // Fill color (red, green, blue)
  double stroke_rgb[3]; // Stroke color (red, green, blue)
  double line_width;  // Line width in points
} graphics_state_t;

// --- Global variables for rendering ---
#define MAX_GSTATE 64 // Maximum nesting of q/Q operators
static graphics_state_t gstack[MAX_GSTATE];
static int  gstack_ptr = 0; // Current position on the stack
#define MAX_OPERANDS 256 // Maximum number of operands for a command
static double operands[MAX_OPERANDS]; // the operand stack
static int num_operands = 0; // Number of operands on the stack
// 
// 'push_gstate()' -- Save teh current graphics state.
//
static void
push_gstate(cairo_t *cr)
{
  if (gstack_ptr < (MAX_GSTATE -1 ))
  {
    cairo_save(cr);
    memcpy(&gstack[gstack_ptr + 1], &gstack[gstack_ptr], sizeof(graphics_state_t));
    gstack_ptr++;
    printf("DEBUG: Saved graphics state (level %d).\n", gstack_ptr);
  }
  else
  {
    fprintf(stderr, "ERROR: Graphics state stack overflow!\n");
  }
}

//
// 'pop_gstate()' - Restore the previous graphics state.
//
static void
pop_gstate(cairo_t *cr)
{
  if (gstack_ptr > 0)
  {
    cairo_restore(cr);
    gstack_ptr--;
    printf("DEBUG: Restored graphics state (level %d). \n", gstack_ptr);
  }
  else
  {
    fprintf(stderr, "ERROR: Graphics state stack underflow!\n");
  }
}


int                                   // 0 - Exit Status
main( int argc,                       // I - Number of command-line args
      char *argv[])                   // I - Command-line arguments
{
  pdfio_file_t  *pdf;
  pdfio_obj_t   *page;
  pdfio_rect_t  mediabox;
  cairo_surface_t *surface;
  cairo_t         *cr;
  int           status = 0;
  char *input_filename = NULL;
  char *output_filename = NULL;
  int opt;
  int pagenum = 1;
  int dpi = 72;

  while ((opt = getopt(argc, argv, "o:p:r:")) != -1 )
  {
    switch (opt)
    {
      case 'o': 
        output_filename = optarg;
        break;

      case 'p':
        pagenum = atoi(optarg);
          break;

      case 'r':
          dpi = atoi(optarg);
          break;

      default: /* '?' */
        fprintf(stderr, "Usage: %s [-o output.png] input.pdf\n", argv[0]);
        return (1);
    }
  }

  if (optind < argc )
  {
    input_filename = argv[optind];
  }

  if (input_filename == NULL || output_filename == NULL)
  {
    fprintf(stderr, "Usage: %s [-o output.png] input.pdf\n", argv[0]);
    return (1);
  }


  // Open the PDF file
  if ((pdf = pdfioFileOpen(input_filename, NULL, NULL, NULL, NULL)) == NULL)
    return (1);

  // Get the first Page...
  if ((page = pdfioFileGetPage(pdf, pagenum - 1)) == NULL)
  {
    fprintf(stderr, "Unable to get page 1 from '%s'.\n", argv[1]);
    pdfioFileClose(pdf);
    return (1);
  }

  // Get the page size....
  pdfioPageGetMediaBox(page, &mediabox);
  printf("Page 1 is %.2f wide and %.2f high.\n", mediabox.x2 - mediabox.x1, mediabox.y2-mediabox.y1);

  double scale = dpi / 72.0;

  // Create a cairo surface and rendering context...
  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (int)((mediabox.x2 - mediabox.x1) * scale ), (int)((mediabox.y2 - mediabox.y1) * scale ));
  cr  = cairo_create(surface);
  
  cairo_scale(cr, scale, scale);

  // Flip the coordinate system to match PDF (origin at bottom-left)
  cairo_translate(cr, 0, mediabox.y2 - mediabox.y1);
  cairo_scale(cr, 1.0, -1.0);

  // Initialize the graphics state stack
  gstack[0].fill_rgb[0] = 0.0; // Black
  gstack[0].fill_rgb[1] = 0.0;
  gstack[0].fill_rgb[2] = 0.0;
  gstack[0].stroke_rgb[0] = 0.0;
  gstack[0].stroke_rgb[1] = 0.0;
  gstack[0].stroke_rgb[2] = 0.0;
  gstack[0].line_width = 1.0;
  gstack_ptr = 0;

  // Set a white background for now..
  cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  cairo_paint(cr);

  // Open the page's content stream..
  pdfio_stream_t *st = pdfioPageOpenStream(page, 0, true);



  if (st)
  {
    char token[1024]; // Token from stream

    puts("\n---- Executing Page 1 Content Stream ---");
    while (pdfioStreamGetToken(st, token, sizeof(token)))
    {
      if (isdigit(token[0]) || token[0] == '.' || token[0] == '-')
      {
        // We have a number, push it on the operand stack
        if (num_operands < MAX_OPERANDS)
          operands[num_operands++] = atof(token);
      }
      else
      {
        // We have a command, process it...
        printf("DEBUG: Executing command '%s' with '%d' operands. \n", token, num_operands);
        if (!strcmp(token, "q"))
        {
          // Save graphics state
          push_gstate(cr);
        }
        else if (!strcmp(token, "Q"))
        {
          // Restore graphics state
          pop_gstate(cr);
        }
        else if (!strcmp(token, "cm"))
        {
          //  Concatenate matrix
          if (num_operands == 6)
          {
            cairo_matrix_t matrix;
            cairo_matrix_init(&matrix, operands[0], operands[1], operands[2], operands[3], operands[4], operands[5]);
            cairo_transform(cr, &matrix);
            printf("DEBUG: Contatenate matrix [%g %g %g %g %g %g].\n", operands[0], operands[1], operands[2], operands[3], operands[4], operands[5]);
          }
        }
        else if (!strcmp(token, "w"))
        {
          // Set line width
          if (num_operands == 1)
          {
            gstack[gstack_ptr].line_width = operands[0];
            cairo_set_line_width(cr, operands[0]);
            printf("DEBUG: Set line width to %g.\n",operands[0]);
          }
        }
        else if (!strcmp(token, "rg"))
        {
          // Set fill Color (RGB)
          if (num_operands == 3)
          {
            gstack[gstack_ptr].fill_rgb[0] = operands[0];
            gstack[gstack_ptr].fill_rgb[1] = operands[1];
            gstack[gstack_ptr].fill_rgb[2] = operands[2];
            cairo_set_source_rgb(cr, operands[0], operands[1], operands[2]);
            printf("DEBUG: Set fill color to %g %g %g \n", operands[0], operands[1], operands[2]);
          }
        }
        else if (!strcmp(token, "RG"))
        {
          // Set Stroke Color (RGB)
          if (num_operands == 3)
          {
            gstack[gstack_ptr].stroke_rgb[0] = operands[0];
            gstack[gstack_ptr].stroke_rgb[1] = operands[1];
            gstack[gstack_ptr].stroke_rgb[2] = operands[2];
            cairo_set_source_rgb(cr, operands[0], operands[1], operands[2]);
            printf("DEBUG: Set stroke color to %g %g %g \n", operands[0], operands[1], operands[2]);
          }
        }
        else if (!strcmp(token, "g"))
        {
          // Set fill Color (Grayscale)
          if (num_operands == 1)
          {
            gstack[gstack_ptr].fill_rgb[0] = operands[0];
            gstack[gstack_ptr].fill_rgb[1] = operands[0];
            gstack[gstack_ptr].fill_rgb[2] = operands[0];
            cairo_set_source_rgb(cr, operands[0], operands[0], operands[0]);
            printf("DEBUG: Set fill color to gray %g\n", operands[0]);
          }
        }
        else if (!strcmp(token, "G"))
        {
          // Set Stroke Color (Grayscale)
          if (num_operands == 1)
          {
            gstack[gstack_ptr].stroke_rgb[0] = operands[0];
            gstack[gstack_ptr].stroke_rgb[1] = operands[0];
            gstack[gstack_ptr].stroke_rgb[2] = operands[0];
            // To set a gray color in Cairo, you set R, G, and B to the same value.
            cairo_set_source_rgb(cr, operands[0], operands[0], operands[0]);
            printf("DEBUG: Set stroke color to gray %g\n", operands[0]);
          }
        }
        else if (!strcmp(token, "m"))
        {
          //Move to 
          if (num_operands == 2)
          {
            cairo_move_to(cr, operands[0], operands[1]);
            printf("DEBUG: Move to (%g, %g). \n", operands[0], operands[1]);
          }
        }
        else if (!strcmp(token, "l"))
        {
          // Line to
          if (num_operands == 2)
          {
            cairo_line_to(cr, operands[0], operands[1]);
            printf("DEBUG: Line to (%g %g).\n", operands[0], operands[1]);
          }
        }
        else if (!strcmp(token, "re"))
        {
          // REctangle
          if  (num_operands == 4)
          {
            cairo_rectangle(cr, operands[0], operands[1], operands[2], operands[3]);
            printf("DEBUG: Rectangle at (%g, %g) size (%g %g) \n", operands[0], operands[1], operands[2], operands[3]);
          }
        }
        else if (!strcmp(token, "h"))
        {
          // CLose Path
          cairo_close_path(cr);
          printf("DEBUG: Close path.\n");
        }
        else if (!strcmp(token, "S"))
        {
          // Stroke Path
          cairo_stroke(cr);
          printf("DEBUG: Stroke path.\n");
        }
        else if (!strcmp(token, "f"))
        {
          // Fill path
          cairo_fill(cr);
          printf("DEBUG:Fill Path.\n");
        }

        // Clear the operand stack for the next command
        num_operands =0;
      }
    }
    puts("--- End of Content Stream ---\n");
    pdfioStreamClose(st);
  }



  // Save to PNG..
  if (cairo_surface_write_to_png(surface, output_filename) != CAIRO_STATUS_SUCCESS)
  {
    fprintf(stderr, "Unable to write to '%s'.\n", argv[2]);
    status = 1;
  }
  else
  {
    printf("Wrote blank page to '%s'.\n", argv[2]);
  }

  //Clean up..
  cairo_destroy(cr);
  cairo_surface_destroy(surface);
  pdfioFileClose(pdf);

  return (status);
}

