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

  // 3. Create a page
  pdfio_stream_t *page_stream = pdfioFileCreatePage(pdf, NULL);
  if (!page_stream)
  {
    fprintf(stderr, "Error: Could not create page in '%s'.\n", output_filename);
    pdfioFileClose(pdf);
    return (1);
  }

// 4. Write PDF commands to draw a donut shape
  pdfioStreamPrintf(page_stream, "q\n");                // Save graphics state
  pdfioStreamPrintf(page_stream, "0.8 0.1 0.1 rg\n");   // Set fill color to red

  // The key to the even-odd rule is the path direction.
  // We will draw the outer circle counter-clockwise and the inner circle clockwise.

  // --- Outer Circle (Counter-Clockwise) ---
  // A circle is drawn with four Bezier curves.
  pdfioStreamPrintf(page_stream, "306 650 m\n"); // Move to top of outer circle
  pdfioStreamPrintf(page_stream, "194 650 100 556 100 444 c\n"); // Top-left quadrant
  pdfioStreamPrintf(page_stream, "100 332 194 238 306 238 c\n"); // Bottom-left quadrant
  pdfioStreamPrintf(page_stream, "418 238 512 332 512 444 c\n"); // Bottom-right quadrant
  pdfioStreamPrintf(page_stream, "512 556 418 650 306 650 c\n"); // Top-right quadrant

  // --- Inner Circle (Clockwise) ---
  pdfioStreamPrintf(page_stream, "306 550 m\n"); // Move to top of inner circle
  pdfioStreamPrintf(page_stream, "362 550 406 506 406 450 c\n"); // Top-right quadrant
  pdfioStreamPrintf(page_stream, "406 394 362 350 306 350 c\n"); // Bottom-right quadrant
  pdfioStreamPrintf(page_stream, "250 350 206 394 206 450 c\n"); // Bottom-left quadrant
  pdfioStreamPrintf(page_stream, "206 506 250 550 306 550 c\n"); // Top-left quadrant

  // Use the even-odd fill operator 'f*' to create the hole
  pdfioStreamPrintf(page_stream, "f*\n");

  pdfioStreamPrintf(page_stream, "Q\n");                // Restore graphics state


  // 5. Close and save the file
  pdfioStreamClose(page_stream);
  pdfioFileClose(pdf);

  printf("Successfully created %s\n", output_filename);

  return (0);
}
