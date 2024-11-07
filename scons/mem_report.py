
import os
import sys


# Path to main project binary file
path_to_binary = sys.argv[1]

##########################################################                
# Size Info For Main Binary              
##########################################################

print('Size Info For Main Binary' + '(' + path_to_binary + '):\n')

stream = os.popen('size ' + path_to_binary)
size_info = stream.read().rstrip()
print(size_info)

########################################################## 
# Size Info For Libraries With Top 5 Size Usage 
##########################################################

if (os.path.isdir('build/arm/bin/libraries')):
  stream = os.popen('size build/arm/bin/libraries/*')
  size_info = stream.readlines()

  # remove the first element (columns)
  columns = size_info[0]
  size_info.pop(0)

  size_info_list = []
  for size in size_info:
      size_info_list.append(size.split(None, 5))

  # sorts by the 'dec' and 'bss' columns
  size_info_dec = sorted(size_info_list, key = lambda x: int(x[3]), reverse=True)
  size_info_bss = sorted(size_info_list, key = lambda x: int(x[2]), reverse=True)

  print("\n\nSize Info for Top 5 Libraries with the Greatest Total Size('dec'):\n")
  for i in range (0,5):
    size_info_dec[i][5] = size_info_dec[i][5].strip()
    print("\t".join(size_info_dec[i]))
    
  print("\n\nSize Info for Libraries with the Greatest Block Starting Symbol('bss'):\n")
  for i in range (0,5):
    size_info_bss[i][5] = size_info_bss[i][5].strip()
    print("\t".join(size_info_bss[i]))
else:
  print("\n\nNo libaries info found")
  
##########################################################
# Display all bss objects with size >= 512  
##########################################################

def split_str(line):
  return line.split()

bss_data = []

with open('build/out.map') as file:
  rows = map(split_str, file)
  for row in rows:
    if (len(row) != 0 and row[0].startswith(".bss.")):
      if (len(row) != 1):
        bss_data.append(row)
      else:
        row.extend(next(rows))
        bss_data.append(row)
      
if (len(bss_data) != 0):
  # sorts bss_data by size of the object 
  def order_by_size(data):
    return int(data[2], 16)

  sorted_bss_data = sorted(bss_data, key=order_by_size, reverse=True)

  print('\n\nBSS Object info with Greatest Total Size (>= 512 bytes):\n')
  for row in sorted_bss_data:
    size = row[2] = int(row[2], 16)
    if (size >= 512):
      print(row[0] + '\n')
      print('\t\t\t\t' + row[1] + '      ' + str(row[2]) + ' ' + row[3] + '\n')
      
else:
  print("\nNo BSS data found\n")
