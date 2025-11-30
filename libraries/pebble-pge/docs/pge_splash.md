# PGE Splash Documentation

`pge_splash.h` allows you to show a simple animated splash screen displaying
that your game was built with this engine. Help spread the word!

![splash](screenshots/screenshot3.png)

1. Create a callback for when the splash screen is done. You should prepare the
   next screen of your game app here. It is recommended to call `pge_begin()`
   here for a smooth transition to your main game view:

        static void after_splash_callback() {
          pge_begin(logic, render, click);
        }

2. Show the splash screen when your app starts:

        static void pge_init() {
          pge_splash_show(after_splash_callback);
        }

When the splash is done, your callback will be called. When the user backs out
of your game, the splash screen will not be displayed again.