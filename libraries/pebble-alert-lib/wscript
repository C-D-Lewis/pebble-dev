top = '.'
out = 'build'

def options(ctx):
    ctx.load('pebble_sdk')

def configure(ctx):
    ctx.load('pebble_sdk')

def build(ctx):
    ctx.load('pebble_sdk')

    ctx.stlib(
    	source = ctx.path.ant_glob('src/**/*.c'), 
    	target = 'alert')
