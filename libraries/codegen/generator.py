from optparse import OptionParser
import jinja2
import yaml

def read_yaml(yaml_file):
    with open(yaml_file, "r") as f:
        data = yaml.load(f, Loader=yaml.FullLoader)
        return data
    
def write_template(env, template_name, data, dest="./"):
    template = env.get_template(template_name)
    output = template.render(data=data)
    file = dest + template_name[:-6]
    with open(file, "w") as f:
        f.write(output)

def main():
    print("Done autogenerating")

if __name__ == "__main__":
    parser = OptionParser()
    parser.add_option("-y", "--yaml_file", default=[], dest="yaml_file", action="append",
                    help="yaml file to read", metavar="FILE")
    parser.add_option("-t", "--template", dest="template",
                    help="template file to populate", metavar="FILE")
    
    (options, args) = parser.parse_args()
    
    templateLoader = jinja2.FileSystemLoader(searchpath="./libraries/codegen/templates")
    env = jinja2.Environment(loader=templateLoader)

    for y in options.yaml_file:
        data = read_yaml(y)    
    
        if options.template:
            write_template(env, options.template, data)
    
    main()
    
