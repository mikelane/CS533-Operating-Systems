<!DOCTYPE html>
<html>
<head>
<link rel="stylesheet" href="/~kstew2/style.css" type="text/css" media="screen" title="no title" charset="utf-8">
<title>M:N Cooperative Round-Robin Scheduler</title>
</head>
<body>
<div id=content>
<h2>M:N Cooperative Round-Robin Scheduler</h2>

<p>One potential design makes almost no changes to the existing scheduler.
There is still just one ready queue (protected with a spinlock), 
and each new processor has an idle thread which continuously yields, pulling
work off the ready queue if it is available. User-level threads are not
preempted involuntairly, though their underlying kernel thread might be.</p>

<p>In the animation below, green rectangles represent running threads, and gray
rectangles represent ready threads. The infinity symbol (&#8734;) represents an
idle thread whose sole job is to <code>yield</code> in an infinite loop.</p>

<div id="loading">
Loading diagram... please wait!
</div>

<div id="image" style="display:none">
  <img src="figure2.gif" style="width:680px">
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
