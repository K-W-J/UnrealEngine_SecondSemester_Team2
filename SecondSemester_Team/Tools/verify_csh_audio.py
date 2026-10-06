import runpy
import sys
from pathlib import Path

sys.argv.append('--verify')
runpy.run_path(str(Path(__file__).with_name('import_csh_audio.py')), run_name='__main__')
