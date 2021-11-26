Rebol [
	Title:   "Rebol3 BBcode test script"
	Author:  "Oldes"
	File: 	 %bbcode-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

import 'bbcode

~~~start-file~~~ "BBcode tests"
if find codecs 'BBCode [
===start-group=== "bbcode function"

--test-- "bbcode"
	test-cases: [
		{text [b]bold[/b] abc}             {<p>text <b>bold</b> abc</p>}
		{text [b]bold [i]italic[/b]}       {<p>text <b>bold <i>italic</i></b></p>}
		{[s]strikethrough text[/s]}        {<p><s>strikethrough text</s></p>}
		{[url]http://example.org[/url]}    {<p><a href="http://example.org">http://example.org</a></p>}
		{[url=http://example]Test[/url]}   {<p><a href="http://example">Test</a></p>}
		{[url=http://example][b]Test[/url]}{<p><a href="http://example"><b>Test</b></a></p>}
		{[url='/bla/ bla.jpg' rel=images]} {<p><a href="/bla/ bla.jpg" rel="images"></a></p>}
		{[b][ul][li]Jenny[/li][li]Alex[/li][li]Beth[/li][/ul][/b]}
										   {<p><b><ul><li>Jenny</li><li>Alex</li><li>Beth</li></ul></b></p>}
		{[ul][li]bla[li]bla}               {<ul><li>bla</li><li>bla</li></ul>}
		{text[ul][li]bla[li]bla}           {<p>text</p><ul><li>bla</li><li>bla</li></ul>}
		{[ul][li][b]bla[li]bla}            {<ul><li><b>bla</b></li><li>bla</li></ul>}
		{[ul][li]bla[li][ol][li]bla[/ol]}  {<ul><li>bla</li><li><ol><li>bla</li></ol></li></ul>}
		{[ul][*]bla^/^/bla} {<ul><li><p>bla</p>^/^/<p>bla</p></li></ul>}
		{[ul][*]bla[*]bla}  {<ul><li>bla</li><li>bla</li></ul>}
		{[code]xx[b]yy[/b]zz[/code]}       {<p><code>xx[b]yy[/b]zz</code></p>}
		{[list][*]aaa[*]bbb[/list]}        {<ul><li>aaa</li><li>bbb</li></ul>}
		{[list=a][*]aaa[*]bbb[/list]}      {<ol style="list-style-type: lower-alpha;"><li>aaa</li><li>bbb</li></ol>}
		{[list=A][*]aaa[*]bbb[/list]}      {<ol style="list-style-type: upper-alpha;"><li>aaa</li><li>bbb</li></ol>}
		{[/b]}                             {<p></p>}
		{[quote]blabla}                    {<p><fieldset><blockquote>blabla</blockquote></fieldset></p>}
		{[quote=Carl]blabla}               {<p><fieldset><legend>Carl</legend><blockquote>blabla</blockquote></fieldset></p>}
		{[img]http://www.google.com/intl/en_ALL/images/logo.gif[/img]}
										   {<p><img src="http://www.google.com/intl/en_ALL/images/logo.gif" alt=""></p>}
		{[url=http://http://www.google.com/][img]http://www.google.com/intl/en_ALL/images/logo.gif[/url][/img]}
										   {<p><a href="http://http://www.google.com/"><img src="http://www.google.com/intl/en_ALL/images/logo.gif" alt=""></a></p>}
		{[img]1.gif [img]2.gif}            {<p><img src="1.gif" alt=""> <img src="2.gif" alt=""></p>}
		{text [size=tiny]tiny}             {<p>text <span style="font-size: xx-small;">tiny</span></p>}
		{[h1]header[/h1]}                  {<h1>header</h1>}
		{[h6]header[/h6]}                  {<h6>header</h6>}
		{[h1]big[h3]smaller[h6]small}      {<h1>big<h3>smaller<h6>small</h6></h3></h1>}
		{[h1]big[h3]smaller[h6]small}      {<h1>big<h3>smaller<h6>small</h6></h3></h1>}
		{[h2]header2^/^/[h4]header4}       {<h2>header2</h2>^/^/<h4>header4</h4>}
		{[color]ee[/color][color=#F00]red[color=#00FF00]green}
										   {<p>ee<span style="color: #F00;">red<span style="color: #00FF00;">green</span></span></p>}
		{<a>}                              {<p>&lt;a></p>}
		{multi^/line}                      {<p>multi^/line</p>}
		{invalid [size]size[/size]}        {<p>invalid <span>size</span></p>}
		{[align=right]right}               {<p><div style="text-align: right;">right</div></p>}
		{[email]x@x.cz[/email] [email=x@x.cz]email [b]me[/email]}
										   {<p><a href="mailto:x@x.cz">x@x.cz</a> <a href="mailto:x@x.cz">email <b>me</b></a></p>}

		{[u]underlined[/u]}                {<p><u>underlined</u></p>}
		{[h3=underline]Portréty [b]X}      {<h3 class="underline">Portréty <b>X</b></h3>}
		{[img=680x300]pozvanka.jpg}        {<p><img width=680 height=300 src="pozvanka.jpg" alt=""></p>}
		{[img width="680"]pozvanka.jpg}        {<p><img width=680 src="pozvanka.jpg" alt=""></p>}
		{[img=680x317 alt=images]pozvanka.jpg} {<p><img width=680 height=317 src="pozvanka.jpg" alt="images"></p>}
		{[img size="680x300" alt='pozvanka na "vystavu"']pozvanka.jpg}
										   {<p><img width=680 height=300 src="pozvanka.jpg" alt="pozvanka na &quot;vystavu&quot;"></p>}
		{[img width="680" height="300" alt="pozvanka na vystavu"]pozvanka.jpg}
										   {<p><img width=680 height=300 src="pozvanka.jpg" alt="pozvanka na vystavu"></p>}
		;{[img resize=680x0]pozvanka.jpg[/]} {<img width=680 height=300 src="pozvanka.jpg" alt="">}
		{[h2=title]stylised title[/h2]}    {<h2 class="title">stylised title</h2>}
		{text^/on one line ^/second line}  {<p>text^/on one line <br>^/second line</p>}
		{par 1^/^/par 2}                   {<p>par 1</p>^/^/<p>par 2</p>}
		{[script][/script]}                {<p>[script]</p>}
		{[url=http://][h1=]bla}            {<p><a href="http://"><h1>bla</h1></a></p>}
		{[ul][*][url]http://test[*]foo[/]} {<ul><li><a href="http://test">http://test</a></li><li>foo</li></ul>}
		{[hr]}                             {<p><hr></p>}
		{[hr 10]}                          {<p><hr style="width:10"></p>}
		{[hr10%]}                          {<p><hr style="width:10%"></p>}
		{[anchor]foo[/anchor]}             {<p><a name="foo"></a></p>}
		{[class=underline]foo}             {<p><span class="underline">foo</span></p>}
	]
	bbcode: :codecs/bbcode/decode
	foreach [src result] test-cases [
		--assert a: equal? result tmp: bbcode src
		unless a [
			print ["==>" mold src]
			print ["<==" mold tmp]
			print ["Exp" mold result]
			print "---"
		]
	]

===end-group===
]
~~~end-file~~~