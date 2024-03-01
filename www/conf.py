# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import glob

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'embedlog'
copyright = '2024, Michał Łyszczek'
author = 'Michał Łyszczek'
release = 'v9999'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
	"sphinx.ext.autosectionlabel",
]

templates_path = ['templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']
autosectionlabel_prefix_document = True

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'alabaster'
html_static_path = ['static']
highlight_language = 'none'


html_css_files = [
	'http://static.bofc.pl/fonts.css',
	'static/custom.css'
]

html_theme_options = {
	'github_user': 'mlyszczek',
	'github_repo': 'embedlog',
	'github_banner': True,
	'github_type': 'star',
	'page_width': '103ch',
	'code_font_family': 'JetBrainsMono',
	'caption_font_family': 'sans',
	'head_font_family': 'sans',
	'font_family': 'sans',
}

man_pages = []
for file in glob.glob('manuals/*/*.[0-9].rst'):
	f = file.split("/")[2]
	man_pages.append((
		file[:-4], # source file (no extension)
		f[:-6].replace('/', '-'), # output file (under output dir)
		'',
		'Michał Łyszczek <michal.lyszczek@boc.pl>',
		f[-5], # section
	))

man_pages.append((
	"el_overview.7",
	"el_overview",
	'',
	'Michał Łyszczek <michal.lyszczek@boc.pl>',
	7
))
