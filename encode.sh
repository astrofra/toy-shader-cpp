ffmpeg -framerate 60 -i img/render/frame_%04d.png -vf "split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse" -loop 0 flopine_full_render.gif
ffmpeg -framerate 60 -i img/render/frame_%04d.png -vf "scale=iw/2:ih/2,select='not(mod(n,2))',split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse" -loop 0 flopine_half_render.gif
