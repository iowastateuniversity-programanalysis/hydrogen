from argparse import ArgumentParser

def get_args():
    parser = ArgumentParser(prog='python hydrogit.py')

    parser.add_argument(
        '-p',
        '--force-pull',
        dest='force_pull',
        help='remove existing versions and pull again',
        default=False,
        )

    parser.add_argument(
        '-b',
        '--force-build',
        dest='force_build',
        help='remove existing versions and build again',
        default=False,
        )

    parser.add_argument(
        '-l',
        '--language',
        dest='language',
        help='compile with this language - should be C or CXX',
        default='CXX',
        )
        
    parser.add_argument('url')
    parser.add_argument('first_version')
    parser.add_argument('latter_versions', nargs='+')

    return parser.parse_args()