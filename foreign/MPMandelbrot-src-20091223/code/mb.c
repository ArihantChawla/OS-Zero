for (int iy = 0 ;iy <height ;iy++) {
    for (int ix = 0 ; ix < width ; ix++) {
        double cx = leftX + ix * stepX;
        double cy = topY - iy * stepY;
        // Z = X+I*Y
        double x = 0;
        double y = 0;
        int it;

        for (it = 0 ; it < itmax ; it++) {
            double x2 = x * x;
            double y2 = y * y;
            // Stop iterations when |Z| > 2

            if (x2 + y2 > 4.0) break;

            double twoxy = 2.0 * x *y;

            // Z = Z^2 + C
            x = x2 - y2 + cx;
            y = twoxy + cy;
        }
        // Here IT is in 0..ITMAX, get the corresponding COLOR
        unsigned int color = mbgetcolor(it, itmax);

        // Store the color in the A array
        a[ix + iy * width] = color;
    }
}

