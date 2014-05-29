<!DOCTYPE html>
<html>
  <head>
    <title>Welcome to ntopng</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link href="/bootstrap/css/bootstrap.css" rel="stylesheet" media="screen">
    <link href="/bootstrap/css/bootstrap-theme.css" rel="stylesheet">
    <link href="/font-awesome/css/font-awesome.css" rel="stylesheet">
    <script src="/js/jquery.js"></script>
    <script src="/bootstrap/js/bootstrap.js"></script>
    <script src="/js/bootstrap-datatable.js"></script>
    <script charset="utf-8" src="/js/d3.v3.js"></script>
    <script src="/js/jquery.peity.js"></script>
    <script src="/js/rickshaw.js"></script>
    <script src="/js/pie-chart.js"></script>
    <script src="/js/gauge.js"></script>
    <link href="/css/flags.css" rel="stylesheet">
    <link href="/css/pie-chart.css" rel="stylesheet">
    <link href="/css/rickshaw.css" rel="stylesheet">
    <!-- http://kamisama.github.io/cal-heatmap/v2/ -->
    <script src="/js/cal-heatmap.js"></script>
    <link href="/css/cal-heatmap.css" rel="stylesheet">
    <script src="/js/springy-ntop.js"></script>
    <script src="/js/springyui-ntop.js"></script>

    <!-- HTML5 shim, for IE6-8 support of HTML5 elements -->
    <!--[if lt IE 9]>
      <script src="/js/html5shiv.js"></script>
    <![endif]-->

<style type="text/css">
  // Media queries breakpoints
  // --------------------------------------------------

  // Extra small screen / phone
  @screen-xsmall:              480px;
  @screen-phone:               @screen-xsmall;

  // Small screen / tablet
  @screen-small:               768px;
  @screen-tablet:              @screen-small;

  // Medium screen / desktop
  @screen-medium:              992px;
  @screen-desktop:             @screen-medium;

  // Large screen / wide desktop
  @screen-large:               1200px;
  @screen-large-desktop:       @screen-large;

  // So media queries don't overlap when required, provide a maximum
  @screen-small-max:           (@screen-medium - 1);
  @screen-tablet-max:          (@screen-desktop - 1);
  @screen-desktop-max:         (@screen-large-desktop - 1);

  // Grid system
  // --------------------------------------------------

  // Number of columns in the grid system
  @grid-columns:              12;
  // Padding, to be divided by two and applied to the left and right of all columns
  @grid-gutter-width:         30px;
  // Point at which the navbar stops collapsing
  @grid-float-breakpoint:     @screen-tablet;

</style>


 <style type="text/css">
      body {
        padding-top: 0px;
        padding-bottom: 20px;
      }

      /* Custom container */
      .container-narrow {
        margin-left: 20px;
        margin-right: 20px;
        /* max-width: 1024px; */
      }

      /* Smaller table row padding */
      .table th, .table td {
        padding: 6px;
      }

      /* Custom container */
      .progress {
        margin-bottom: 0px;
      }
      .container-narrow > hr {
        margin: 30px 0;
      }

      /* Main marketing message and sign up button */
      .jumbotron {
        margin: 20px 0;
        text-align: center;
      }
      .jumbotron h1 {
        font-size: 22px;
        line-height: 1;
      }
      .jumbotron .btn {
        font-size: 21px;
        padding: 14px 24px;
      }

      /* Supporting marketing content */
      .marketing {
        margin: 20px 0;
      }
      .marketing p + h4 {
        margin-top: 28px;
      }

      .table th.text-right,
      .table td.text-right {
        text-align: right;
      }

      .navbar-nav > li > a {padding-top:10px !important; padding-bottom:10px !important;}
      .navbar {min-height:42px !important}
      .nav-pills {min-height:42px !important}

/* Fix for Google Maps and Twitter Bootstrap
      https://github.com/twbs/bootstrap/issues/1552
*/
img {
  max-width: none;
}
    </style>
  </head>
<body>

  <div class="container-narrow">


