<!DOCTYPE html>
<html>
  <head>
    <title>Welcome to ntopng</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link href="@HTTP_PREFIX@/bootstrap/css/bootstrap.css" rel="stylesheet">
    <script src="@HTTP_PREFIX@/js/jquery.js"></script>
    <script src="@HTTP_PREFIX@/bootstrap/js/bootstrap.js"></script>
    <script src="@HTTP_PREFIX@/js/bootstrap-datatable.js"></script>
    <script charset="utf-8" src="@HTTP_PREFIX@/js/d3.v3.js"></script>
    <script src="@HTTP_PREFIX@/js/jquery.peity.js"></script>
    <script src="@HTTP_PREFIX@/js/pie-chart.js"></script>
    <link href="@HTTP_PREFIX@/css/flags.css" rel="stylesheet">
    <link href="@HTTP_PREFIX@/css/pie-chart.css" rel="stylesheet">
    <link href="@HTTP_PREFIX@/css/rickshaw.css" rel="stylesheet">

    <!-- HTML5 shim, for IE6-8 support of HTML5 elements -->
    <!--[if lt IE 9]>
      <script src="@HTTP_PREFIX@/js/html5shiv.js"></script>
    <![endif]-->



 
  </head>
<body>

  <div class="container-narrow">



 <style type="text/css">
      body {
        padding-top: 40px;
        padding-bottom: 40px;
        background-color: #f5f5f5;
   }

      .form-signin {
        max-width: 350px;
        padding: 9px 29px 29px;
        margin: 0 auto 20px;
        background-color: #fff;
        border: 1px solid #e5e5e5;
        -webkit-border-radius: 5px;
           -moz-border-radius: 5px;
                border-radius: 5px;
          -webkit-box-shadow: 0 1px 2px rgba(0,0,0,.05);
       -moz-box-shadow: 0 1px 2px rgba(0,0,0,.05);
      box-shadow: 0 1px 2px rgba(0,0,0,.05);
   }
      .form-signin .form-signin-heading,
      .form-signin .checkbox {
        margin-bottom: 10px;
      }
      .form-signin input[type="text"],
      .form-signin input[type="password"] {
        font-size: 16px;
        height: auto;
        margin-bottom: 15px;
        padding: 7px 9px;
      }

    </style>

<div class="container">

  <form class="form-signin" action="@HTTP_PREFIX@/authorize.html" method="POST">
    <h2 class="form-signin-heading" style="font-weight: bold;">Welcome to ntopng</h2>
    <input type="text" class="form-control" name="user" placeholder="Username">
    <input type="password" class="form-control" name="password" placeholder="Password">
    <button class="btn btn-lg btn-primary btn-block" type="submit">Login</button>
  	<div class="row">
      <div >&nbsp;</div>
      <div class="col-lg-12">
        <small>
          <p>If you find ntopng useful, please support us by making a small <A href=http://shop.ntop.org>donation</A>. Your funding will help to run and foster the development of this project. Thank you.
          </p>

          <p>&copy; ntop.org - ntopng is released under <A HREF=http://www.gnu.org/copyleft/gpl.html>GPLv3</A>.</p>

  	      <p class="text-muted">Hint: the default user and password are admin</p>
        </small>
      </div>
    </div>
  </form>

</div> <!-- /container -->

</body>
</html>

