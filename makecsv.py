with open('sowpods.csv', 'w') as outfile:
	with open('sowpods.txt', 'r') as infile:
	    for line_index, line in enumerate(infile):
	        outfile.write('{0},{1}\n'.format(line.strip(), line_index))
