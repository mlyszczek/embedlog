#!/bin/sh

# add single new line before about in index.html
sed -i 's/<h1 id="about">/<h1 id="about" class="first">/' "out/index.html"
