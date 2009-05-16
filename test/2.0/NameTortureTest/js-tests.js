function CreateFromXamlWithNamescope ()
{
  var content = document.getElementById ("silverlightControl").content;
  var Assert = content.Assert;

  this.RunTest = function () {
    // root of loaded tree is a FWE
    var c = content.createFromXaml (
'<Canvas \
    xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" \
    xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml" > \
  <Canvas.Background> \
    <SolidColorBrush x:Name="brush" /> \
  </Canvas.Background> \
  <Border x:Name="border" /> \
  <TextBlock Name="text"> \
    <Run Name="run1" /> \
    <Run x:Name="run2" /> \
  </TextBlock> \
</Canvas> \
', true);
    // we can use FindName on the toplevel FWE to find all objects created with x:Names in the xaml
    Assert.IsNotNull (c.findName ("brush"), "1");
    Assert.IsNotNull (c.findName ("border"), "2");
    Assert.IsNotNull (c.findName ("run2"), "3");

    // and also those created with Name where appropriate
    Assert.IsNotNull (c.findName ("text"), "4");
    Assert.IsNotNull (c.findName ("run1"), "5");

    // root of loaded tree is not a FWE
    var rd = content.createFromXaml (
'<ResourceDictionary  \
    xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" \
    xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml" > \
  <Border x:Name="border" /> \
  <Storyboard x:Name="hi" /> \
</ResourceDictionary> \
');

    Assert.IsNull (rd["border"], "6");
    Assert.IsNotNull (rd.getItem ("border"), "7");
    Assert.IsNotNull (rd.getItem ("border").findName("hi"), "8");
  };
}


function ContentFindNameFallback ()
{
  var content = document.getElementById ("silverlightControl").content;
  var Assert = content.Assert;

  this.RunTest = function () {
    // root of loaded tree is a FWE
    var c = content.createFromXaml (
'<Canvas \
    xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" \
    xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml" > \
  <Canvas.Background> \
    <SolidColorBrush x:Name="brush" /> \
  </Canvas.Background> \
  <Border x:Name="border" /> \
  <TextBlock Name="text"> \
    <Run Name="run1" /> \
    <Run x:Name="run2" /> \
  </TextBlock> \
</Canvas> \
');

    // this causes the canvas's temporary namescope to be merged in with the root
    content.root.findName ("testArea").children.add (c);

    var c2 = content.createFromXaml (
'<Canvas \
    xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" \
    xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml" > \
</Canvas>', true);

    // c2 has its own namescope so we don't merge with the root when adding it
    content.root.findName ("testArea").children.remove (c2);

    // lookup from the initial canvas
    Assert.IsNotNull (c.findName("brush"), "1");

    // lookup from the content
    Assert.IsNotNull (content.findName("brush"), "2");

    // lookup from the second added canvas to see if we fallback to content-level findName.
    Assert.IsNull (c2.findName("brush"), "3");

    // clean up
    content.root.findName ("testArea").children.remove (c);
    content.root.findName ("testArea").children.remove (c2);
  };

}

function FindNameEnclosingNamescopes ()
{
  var content = document.getElementById ("silverlightControl").content;
  var Assert = content.Assert;

  this.RunTest = function () {
    var c = content.createFromXaml (
'<Canvas \
    xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" \
    xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml" > \
      <Border x:Name="border" /> \
</Canvas>', true);


    var c2 = content.createFromXaml (
'<Canvas \
    xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" \
    xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml" > \
</Canvas>', true);

    c.children.add (c2);

    // lookup from the initial canvas
    Assert.IsNotNull (c.findName("border"), "1");

    // lookup from the child canvas
    Assert.IsNull (c2.findName("border"), "2");
  };

}