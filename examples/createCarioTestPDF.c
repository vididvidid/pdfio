#include <stdio.h>
#include "pdfio.h"

//
// 'main()' - Create a PDF with nested, independently-styled boxes.
//
int main(int argc, char *argv[])
{
  // 1. Check for command-line argument
  if (argc != 2)
  {
    fprintf(stderr, "Usage: %s <output-path.pdf>\n", argv[0]);
    return (1);
  }
  const char *output_filename = argv[1];

  // Page and box dimensions
  pdfio_rect_t media_box = {0.0, 0.0, 612.0, 792.0};

  // 2. Create the PDF file
  pdfio_file_t *pdf = pdfioFileCreate(output_filename, NULL, &media_box, NULL, NULL, NULL);
  if (!pdf)
  {
    fprintf(stderr, "Error: Could not create PDF file '%s'.\n", output_filename);
    return (1);
  }
  // 3. Create a resources dictionary for our page
  pdfio_dict_t *resources = pdfioDictCreate(pdf);
  pdfio_dict_t *ext_g_state = pdfioDictCreate(pdf);
  pdfio_dict_t *gs_dict = pdfioDictCreate(pdf);

  // Define a graphics state: line width (LW) of 10 points
  pdfioDictSetNumber(gs_dict, "LW", 10.0);

  // Add this graphics state to the ExtGState dictionary with the name "GS1"
  pdfioDictSetDict(ext_g_state, "GS1", gs_dict);

  // Add the ExtGState dictionary to the main resources dictionary
  pdfioDictSetDict(resources, "ExtGState", ext_g_state);

  // 4. Create a page and associate our resources with it
  pdfio_stream_t *page_stream = pdfioFileCreatePage(pdf, resources);
  if (!page_stream)
  {
    fprintf(stderr, "Error: Could not create page in '%s'.\n", output_filename);
    pdfioFileClose(pdf);
    return (1);
  }

  // 5. Write PDF commands to draw a shape using the graphics state
  pdfioStreamPrintf(page_stream, "q\n");                // Save graphics state

  // Set the stroke color to blue
  pdfioStreamPrintf(page_stream, "0 0 1 RG\n");

  // Use the 'gs' operator to apply the "GS1" graphics state (which sets line width to 10)
  pdfioStreamPrintf(page_stream, "/GS1 gs\n");

  // Draw a rectangle
  pdfioStreamPrintf(page_stream, "100 500 412 200 re\n");

  // Stroke the path. It should be drawn with a 10-point thick line.
  pdfioStreamPrintf(page_stream, "S\n");

  pdfioStreamPrintf(page_stream, "Q\n");                // Restore graphics state

  // 5. Close and save the file
  pdfioStreamClose(page_stream);
  pdfioFileClose(pdf);

  printf("Successfully created %s\n", output_filename);

  return (0);
}
