import jinja2

x = jinja2.Template("""
{{ '0x%016x' % ('1' * 0 + '0' * 8 + '1' * (64 - 8 - 8)) | int(base=2) }}
""")

print(x.render())
