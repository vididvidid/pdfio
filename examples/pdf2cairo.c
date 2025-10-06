#include "pdfio.h"
#include <cairo/cairo.h>
#include <stdio.h>
#include <string.h>

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

  if (argc != 3)
  {
    puts("Usage: ./pdf2cairo input.pdf output.png");
    return (1);
  }

  // Open the PDF file
  if ((pdf = pdfioFileOpen(argv[1], NULL, NULL, NULL, NULL)) == NULL)
    return (1);

  // Get the first Page...
  if ((page = pdfioFileGetPage(pdf, 0)) == NULL)
  {
    fprintf(stderr, "Unable to get page 1 from '%s'.\n", argv[1]);
    pdfioFileClose(pdf);
    return (1);
  }

  // Get the page size....
  pdfioPageGetMediaBox(page, &mediabox);
  printf("Page 1 is %.2f wide and %.2f high.\n", mediabox.x2 - mediabox.x1, mediabox.y2-mediabox.y1);

  // Open the page's content stream..
  pdfio_stream_t *st = pdfioPageOpenStream(page, 0, true);
  if (st)
  {
    char token[1024]; // A buffer to hold the token we read
    puts("\n--- Page 1 Content Stream Tokens ---");
    while (pdfioStreamGetToken(st, token, sizeof(token)))
    {
      printf("TOKEN: '%s'\n", token);
    }
    puts("--- Ends of Tokens ---\n");
    pdfioStreamClose(st);
  }


  // Create a cairo surface and rendering context...
  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (int)(mediabox.x2 - mediabox.x1), (int)(mediabox.y2 - mediabox.y1));
  cr  = cairo_create(surface);

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

  // Save to PNG..
  if (cairo_surface_write_to_png(surface, argv[2]) != CAIRO_STATUS_SUCCESS)
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

