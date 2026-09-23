# It looks like shit, but I'm not particularly worried about such scripts.

import os
import re

condition_token = re.compile(r'\s*(&&|\|\||!|\(|\)|\$?[a-zA-Z0-9_.]+)')

match_statement = re.compile(r'\[.*\]')

def compute_statement( defines, statement ):
	enabled = {define.split('=')[0] for define in defines}
	expression = statement.strip()
	if not expression.startswith('[') or not expression.endswith(']'):
		raise ValueError('invalid VPC condition: ' + statement)
	expression = expression[1:-1]
	tokens = []
	position = 0
	while position < len(expression):
		if expression[position:].isspace():
			break
		match = condition_token.match(expression, position)
		if match is None:
			raise ValueError('invalid VPC condition: ' + statement)
		tokens.append(match.group(1))
		position = match.end()
	index = 0

	def atom():
		nonlocal index
		if index >= len(tokens):
			raise ValueError('incomplete VPC condition: ' + statement)
		token = tokens[index]
		index += 1
		if token == '!':
			return not atom()
		if token == '(':
			value = or_expression()
			if index >= len(tokens) or tokens[index] != ')':
				raise ValueError('unclosed VPC condition: ' + statement)
			index += 1
			return value
		if token in (')', '&&', '||'):
			raise ValueError('invalid VPC condition: ' + statement)
		name = token.lstrip('$')
		return name == '1' or (name != '0' and name in enabled)

	def and_expression():
		nonlocal index
		value = atom()
		while index < len(tokens) and tokens[index] == '&&':
			index += 1
			right = atom()
			value = value and right
		return value

	def or_expression():
		nonlocal index
		value = and_expression()
		while index < len(tokens) and tokens[index] == '||':
			index += 1
			right = and_expression()
			value = value or right
		return value

	result = or_expression()
	if index != len(tokens):
		raise ValueError('invalid VPC condition: ' + statement)
	return result

def project_key(l):
	for k in l.keys():
		if '$Project' in k:
			return k

def fix_dos_path( path ):
	path = path.replace('\\', '/')
	p = path.split('/')

	filename = p[-1]
	find_path = '/'.join(p[0:len(p)-1])
	if find_path == '': find_path = './'
	else: find_path += '/'

	if not os.path.exists(find_path):
		return find_path+filename

	dirlist = os.listdir(find_path)
	for file in dirlist:
		if file == filename:
			return find_path+file
		elif file.lower() == filename.lower():
			return find_path+file
	return find_path+filename

def parse_vpcs( env ,vpcs, basedir ):
	back_path = os.path.abspath('.')
	os.chdir(env.SUBPROJECT_PATH[0])

	sources = []
	defines = []
	includes = []

	for vpc in vpcs:
		with open(vpc, 'r') as stream:
			f = stream.read().replace('\\\n', ';')

		l = f.split('\n')

		iBrackets = 0

		next_br = False
		ret = {}
		cur_key = ''
		active_scopes = [True]
		pending_scope = True

		for line_number, i in enumerate(l):
			stripped = i.strip()
			if not stripped or stripped.startswith('//'):
				continue
			if stripped == '{':
				iBrackets += 1
				active_scopes.append(active_scopes[-1] and pending_scope)
				pending_scope = True
				continue
			if stripped == '}':
				iBrackets -= 1
				active_scopes.pop()
				continue

			s = match_statement.search(i)
			selected = not s or compute_statement(env.DEFINES+defines, s.group(0))
			for following in l[line_number + 1:]:
				if following.strip() and not following.strip().startswith('//'):
					if following.strip() == '{':
						pending_scope = selected
					break
			if not active_scopes[-1] or not selected:
				continue

			if i.startswith('$') and iBrackets == 0:
				ret.update({i:[]})
				cur_key = i
				next_br = True
			elif iBrackets > 0:
				ret[cur_key].append(i)

			if next_br:
				next_br = False

		key = project_key(ret)
		l=ret[key]

		for i in l:
			if '-$File' in i and '.h"' not in i:
				for k in i.split(';'):
					k = k.replace('$SRCDIR', basedir)
					s = fix_dos_path(k.split('"')[1])

					for j in range(len(sources)):
						if sources[j] == s:
							del sources[j]
							break

			elif '$File' in i and '.h"' not in i:
				for j in i.split(';'):
					j = j.replace('$SRCDIR', basedir)
					s = fix_dos_path(j.split('"')[1])
					sources.append(s)

		for i in ret['$Configuration']:
			if '$PreprocessorDefinitions' in i:
				i = i.replace('$BASE', '')
				s = i.split('"')[1]
				s = re.split(';|,', s)
				for j in s:
					if j != '' and j not in defines:
						defines.append(j)
			if '$AdditionalIncludeDirectories' in i:
				i = i.replace('$BASE', '').replace('$SRCDIR', basedir)
				s = i.split('"')[1]
				s = re.split(';|,', s)
				for j in s:
					j = j.replace('\\','/')
					if j != '' and j not in includes:
						includes.append(j)
	os.chdir(back_path)

	return {'defines':defines, 'includes':includes, 'sources': sources}
