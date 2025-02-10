# news-headlines

Code for the News Headlines app powered by (but not affiliated with) BBC News.

Open source since December 2016.


## Notes

* The backend server posts pins to subscribers every INTERVAL. It employs a
  duplicate detection mechanism to prevent re-posted stories appearing as
  duplicate pins, as well as to prevent pin spam when the server is launched.

* The app used to support news story thumbnails (requested from, and converted
  by the backend).  When the feed images stopped always being 144x81, they went
  away. Some code exists to support this feature in the future, some is commented
  out, and some may have disappeared.

* The app also used to support opening a pin to see more details as well as pin
  notifications, but these also went away. Same rules apply.


## TODO

* More detailed documentation