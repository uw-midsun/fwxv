import jinja2

x = jinja2.Template("""
{{ '0x%0x' % ('1' * 8 + '0' * 8 + '1' * (64 - 8 - 8)) | int(base=2) }}

asldfkjwef {#- comment -#}
saldfkjwlea {#- comment -#}
lkerew
""")

print(x.render())
