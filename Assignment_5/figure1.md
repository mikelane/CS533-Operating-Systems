<!DOCTYPE html>
<html>
<head>
<link rel="stylesheet" href="/~kstew2/style.css" type="text/css" media="screen" title="no title" charset="utf-8">
<title>N:1 Cooperative Round-Robin Scheduler</title>
</head>
<body>
<div id=content>
<h2>N:1 Cooperative Round-Robin Scheduler</h2>

<p>In an N:1 scheduler, a single kernel thread is multiplex to run
many user-level threads. In a cooperative system like ours,
the current user-level thread is only switched out when it calls <code>yield</code>
(or blocks). Likewise, a thread on the ready list is only switched in when some
other thread yields (or blocks).</p>

<p>In the animation below, green rectangles represent running threads, and gray
rectangles represent ready threads.</p>

<div id="loading">
Loading diagram... please wait!
</div>

<div id="image" style="display:none">
  <img src="figure1.gif" style="width:680px">
</div>

<script type="text/javascript">

document.addEventListener("DOMContentLoaded", function(event) {
  document.getElementById("loading").style.display = 'none';
  document.getElementById("image").style.display = 'block';
});
</script>

</div>
</body>
</html>
