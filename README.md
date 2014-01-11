ml_amptube
==========

ml_amptube is a Winamp media libray plugin. It allows you to search for YouTube videos inside of Winamp and put them into your active playlist.The videos are cached locally and the plugin tells Winamp to play this local files.

TODO
====

The plugin is still in early development and so there are many things that need to be done.

Must have:
  - Make the search result list look nicer
  - Retrieve thumbnail images for search results (cache them too?)
  - Video download (quality selection?)
  - Logic for "add to playlist"
  - Installer for the plugin

Maybe:
  - video streaming instead of download
  
Things to keep in mind
======================

 - The free edition of Winamp (as of version 5.666) only plays sound for MP4 videos. FLV videos work fine out of the box. To enable the video playback too, you either uprade to Winamp Pro or you set up a DirectShow decoder to work with Winamp. I will add a guide here how to do this easily in the near future.
