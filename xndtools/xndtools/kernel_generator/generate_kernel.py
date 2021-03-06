""" Provides: generate_kernel, get_module_data.
"""
# Author: Pearu Peterson
# Created: April 2018

import os
import sys
import re
from glob import glob
from copy import deepcopy
from collections import defaultdict
import pprint
from .readers import PrototypeReader, load_kernel_config
from .utils import NormalizedTypeMap, split_expression, intent_names, prettify
from .kernel_source_template import source_template

def update_argument_maps(expr, depends_map, values_map, shapes_map, arguments):
    if isinstance(expr, tuple): # (<name>, <value|shape>)
        name, value = expr
        for n in re.findall(r'(\b[a-zA-Z_]\w*\b)', value):
            if n in arguments:
                if name not in depends_map[n]: # avoid circular dependencies
                    depends_map[name].add(n)
    elif '=' in expr: # <name>=<expr>
        name, value = expr.split('=', 1)
        name = name.strip()
        update_argument_maps((name, value), depends_map, values_map, shapes_map, arguments)
        assert name not in values_map,repr((name, value, values_map[name]))
        values_map[name] = value
    elif '(' in expr: # <name>(<shape-list>)
        i = expr.index('(')
        if expr[-1] !=')':
            raise ValueError('cannot determine shape from {!r}. IGNORING.'.format(expr))
        name = expr[:i].strip()
        shape = [a.strip() for a in expr[i+1:-1].split(',')]
        shapes_map[name] = shape
    else: # <name>
        name = expr
    return name


def apply_typemap(prototype, typemap, typemap_tests):
    orig_type = prototype['type']
    prototype['type'] = normal_type = typemap(orig_type)
    prototype['ctype'] = c_type = normal_type + '_t'
    zero = typemap.get_zero(normal_type)
    if zero is not None:
        prototype['ctype_zero'] = zero
    if orig_type != normal_type:
        typemap_tests.add((orig_type, c_type))
    
    for arg in prototype['arguments']:
        orig_type = arg['type']
        arg['type'] = normal_type = typemap(orig_type)
        arg['ctype'] = c_type = normal_type + '_t'
        zero = typemap.get_zero(normal_type)
        if zero is not None:
            arg['ctype_zero'] = zero
        if orig_type != normal_type:
            typemap_tests.add((orig_type, c_type))
    
def generate_kernel(config_file,
                    target_file = None,
                    source_dir = ''):
    data = get_module_data(config_file)
    source = source_template(data)
    own_target_file = False
    if target_file == 'stdout':
        target_file = sys.stdout
        own_target_file = False
    else:
        if target_file is None:
            target_file = os.path.join(source_dir, '{module_name}-kernels.c'.format(**data))
        if isinstance(target_file, str):
            print('generate_kernel: kernel sources are saved to {}'.format(target_file))
            target_file = open(target_file, 'w')
            own_target_file = True
        else:
            own_target_file = False

    target_file.write(prettify(source['c_source'], target='c'))
    if own_target_file:
        target_file.close()
    return dict(config_file = config_file,
                sources = [target_file.name] + data['sources'])

def get_module_data(config_file, package=None):
    config = load_kernel_config(config_file)
    reader = PrototypeReader()    
    current_module = None
    xndtools_datadir = os.path.dirname(__file__)
    include_dirs = [xndtools_datadir]
    sources = list(glob(os.path.join(xndtools_datadir, '*.c')))
    kernels = []
    typemap_tests = set()

    default_debug_value = False
    default_kinds_value = 'Xnd' # TODO: move to command line options
    default_ellipses_value = '...'
    default_arraytypes_value = 'symbolic'
    
    for section in config.sections():
        if section.startswith('MODULE'):
            assert current_module is None
            module_name = section.split(None, 1)[1]
            current_module = config[section]

            typemap = NormalizedTypeMap()
            for line in current_module.get('typemaps', '').splitlines():
                line = line.strip()
                if not line or line.startswith('#'):
                    continue
                left, right = line.split(':', 1)
                typemap[left.strip()] = right.strip()

            for line in current_module.get('include_dirs', '').splitlines():
                line = line.strip()
                if not line or line.startswith('#'):
                    continue
                include_dirs.append(line)

            for line in current_module.get('sources', '').splitlines():
                line = line.strip()
                if not line or line.startswith('#'):
                    continue
                sources.append(line)

            default_debug = bool(current_module.get('debug', default_debug_value))
            default_kinds = split_expression(current_module.get('kinds', default_kinds_value))
            default_ellipses = split_expression(current_module.get('ellipses', default_ellipses_value))
            default_arraytypes = split_expression(current_module.get('arraytypes', default_arraytypes_value))
            
        elif section.startswith('KERNEL'):
            f = config[section]
            if f.get('skip', None):
                #print ('skipping', section)
                continue
            kernel_name = section.split(maxsplit=1)[1].strip()
            description = f.get('description','').strip()

            prototypes = reader(f.get('prototypes',''))
            prototypes_C = reader(f.get('prototypes_C', ''))
            prototypes_Fortran = reader(f.get('prototypes_Fortran',''))

            if not (prototypes or prototypes_C or prototypes_Fortran):
                print('get_module_data: no prototypes|prototypes_C|prototypes_Fortran defined in [KERNEL {}]'.format(kernel_name))
                continue

            debug = bool(f.get('debug', default_debug))
            kinds = split_expression(f.get('kinds', ''))
            ellipses = f.get('ellipses')
            if ellipses is None:
                ellipses = default_ellipses
            else:
                ellipses = split_expression(ellipses)
            arraytypes = split_expression(f.get('arraytypes', '')) or default_arraytypes

            assert set(arraytypes).issubset(['symbolic', 'variable']),repr(arraytypes)

            # get argument intents and shape information
            intent_arguments = {}
            for intent_name in intent_names:
                intent_arguments[intent_name] = split_expression(f.get(intent_name+'_arguments', ''))
            argument_dimensions = split_expression(f.get('dimension', ''))

            # propagate prototypes to kernels
            for prototypes_, kinds_ in [
                    (prototypes_C, ['C']),
                    (prototypes_Fortran, ['Fortran']),
                    (prototypes, kinds or default_kinds),
            ]:
                for prototype in prototypes_:
                    prototype['kernel_name'] = kernel_name
                    prototype['description'] = description
                    prototype['function_name'] = prototype.pop('name')
                    prototype['debug'] = debug
                    prototype['oneline_description'] = prototype['description'].lstrip().split('\n',1)[0] or '<description not specified>'
                    apply_typemap(prototype, typemap, typemap_tests)

                    depends_map = defaultdict(set)
                    values_map = {}
                    shapes_map = {}
                    arguments = list(prototype['argument_map'])
                    
                    for intent_name in intent_arguments:
                        for name in intent_arguments[intent_name]:
                            name = update_argument_maps(name, depends_map, values_map, shapes_map, arguments)
                            prototype.set_argument_intent(name, intent_name)

                    for name_shape in argument_dimensions:
                        name = update_argument_maps(name_shape, depends_map, values_map, shapes_map, arguments)

                    max_rank = 0
                    for name, shapes in shapes_map.items():
                        for shape in shapes:
                            update_argument_maps((name, shape), depends_map, values_map, shapes_map, arguments)
                        prototype.set_argument_shape(name, shapes)
                        arg = prototype.get_argument(name)
                        if not arg.is_intent_hide:
                            max_rank = max(max_rank, len(shapes))
                    prototype['max_rank'] = max_rank
                    
                    for name, depends in depends_map.items():
                        prototype.set_argument_depends(name, depends)
                        
                    for name, value in values_map.items():
                        prototype.set_argument_value(name, value)

                    input_args, output_args = prototype.get_input_output_arguments()

                    for arraytype in arraytypes:
                        for kind in kinds_:
                            if arraytype == 'variable' and kind != 'Xnd':
                                continue
                            for ellipses_ in ellipses:
                                kernel = deepcopy(prototype)
                                kernel['kind'] = kind
                                kernel['arraytype'] = arraytype
                                if ellipses_ and ellipses_.lower() != 'none':
                                    if not input_args: # `void -> ... * T` not allowed
                                        continue
                                    if ellipses_ == '...' and arraytype == 'variable':
                                        kernel['ellipses'] = 'var' + ellipses_ + ' * '
                                    else:
                                        kernel['ellipses'] = ellipses_ + ' * '
                                else:
                                    kernel['ellipses'] = ''
                                kernel['ellipses_name'] = kernel['ellipses'].replace('...','_DOTS_').replace('.','_DOT_').replace('*','_STAR_').replace(' ','')
                                kernel['kernel_repr'] = pprint.pformat(kernel, indent=4, compact=True)
                                kernels.append(kernel)

    l = []
    for h in current_module.get('includes','').split():
        h = h.strip()
        if h:
            l.append('#include "{}"'.format(h))
            
    module_data = dict(
        module_name = module_name,
        includes = '\n'.join(l),
        #header_code = current_module.get('header_code', ''),
        include_dirs = include_dirs,
        sources = sources,
        kernels = kernels,
        typemap_tests = list([dict(orig_type=o[0], normal_type=o[1]) for o in typemap_tests]),
    )

    return module_data
