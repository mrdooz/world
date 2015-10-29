import argparse
from string import Template
from PIL import Image, ImageDraw, ImageColor

parser = argparse.ArgumentParser()
parser.add_argument('filename')
parser.add_argument('--sprite-width', '-x', type=int, required=True)
parser.add_argument('--sprite-height', '-y', type=int, required=True)
parser.add_argument('--ofs-x', type=int, default=0)
parser.add_argument('--ofs-y', type=int, default=0)
args = parser.parse_args()
sprite_width = args.sprite_width
sprite_height = args.sprite_height
ofs_x = args.ofs_x
ofs_y = args.ofs_y

im = Image.open(args.filename)
img_width, img_height = im.size

sheet_template = """\
sheet: {
    filename: '$filename';
    size: $width, $height;
};
"""

sprite_template = """\
sprite: {
    name: '$name';
    offset: $x, $y;
    size: $w, $h;
    repeat: 1;
    spacing: 0;
};
"""


t = Template(sheet_template)
print t.substitute(
    {'filename': args.filename, 'width': img_width, 'height': img_height})

s = Template(sprite_template)
idx = 0
for cur_y in range(ofs_y, img_height - sprite_height, sprite_height):
    for cur_x in range(ofs_x, img_width - sprite_width, sprite_width):
        print s.substitute({
            'name': ('sprite_%.4d' % idx),
            'x': cur_x,
            'y': cur_y,
            'w': sprite_width,
            'h': sprite_height,
        })

        idx += 1
