from optparse import OptionParser
import jinja2
import yaml

def read_yaml(yaml_file):
    with open(yaml_file, "r") as f:
        data = yaml.load(f, Loader=yaml.FullLoader)
        return data

def get_file_path(template_name, yaml_path, data, dest="./"):
    # get the name of the yaml file from the filepath
    file_prefix = yaml_path.split("/")[len(yaml_path.split("/"))-1].split(".")[0]
    data['FilePrefix'] = file_prefix
    # get the name of the jinja file from the filepath
    jinja_prefix = template_name[:-6]
    # files that start with _ are generic and we want to append the specific name in front
    return dest + (file_prefix + jinja_prefix if jinja_prefix[0] == "_" else jinja_prefix)
    
def write_template(env, template_name, file_path, data):
    template = env.get_template(template_name)
    output = template.render(data=data)
    with open(file_path, "w") as f:
        f.write(output)

def main():
    print("Done autogenerating")

if __name__ == "__main__":
    parser = OptionParser()
    parser.add_option("-y", "--yaml_file", default=[], dest="yaml_file", action="append",
                    help="yaml file to read", metavar="FILE")
    parser.add_option("-t", "--template", dest="template",
                    help="template file to populate", metavar="FILE")
    parser.add_option("-b", "--board", dest="board",
                    help="which board to generate")
    
    (options, args) = parser.parse_args()
    
    templateLoader = jinja2.FileSystemLoader(searchpath="./libraries/codegen/templates")
    env = jinja2.Environment(loader=templateLoader)

    for y in options.yaml_file:
        data = read_yaml(y)

        if options.template:
            file_path = get_file_path(options.template, y, data)
            write_template(env, options.template, file_path, data)
    
    main()
    
