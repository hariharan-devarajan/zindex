import zindex_py as zindex
import sys
import os
dir = sys.argv[1]
gzip_file = f"{dir}/test.pfw.gz"
gz_index_file_actual = f"{gzip_file}.zindex"
gz_index_file = f"file:{gz_index_file_actual}"

if os.path.exists(gz_index_file_actual):
    os.remove(gz_index_file_actual)

status = zindex.create_index(gzip_file, debug=True)
assert status == 0
assert os.path.exists(gz_index_file_actual)
os.remove(gz_index_file_actual)
assert not os.path.exists(gz_index_file_actual)
status = zindex.create_index(gzip_file, index_file=gz_index_file,
                             regex="id:\b([0-9]+)", numeric=True,
                             unique=True, debug=True)
assert status == 0
assert os.path.exists(gz_index_file_actual)
line_numbers = zindex.get_max_line(gzip_file, index_file=gz_index_file_actual)
assert line_numbers == 16
lines = zindex.zquery(gzip_file, index_file=gz_index_file_actual,
                      raw="select a.line from LineOffsets a;", debug=True)
print(lines)
assert len(lines) == 16
assert lines[0] == "["

size = zindex.get_total_size(gzip_file, index_file=gz_index_file_actual)
print(size)
os.remove(gz_index_file_actual)


