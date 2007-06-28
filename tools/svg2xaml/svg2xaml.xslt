<?xml version="1.0" encoding="UTF-8"?>

<!--
//
// svg2xaml.xslt
//
// This stylesheet converts svg to xaml. It makes use of the 
// node-set extension function to map node fragments, and is
// therefore suited to be run under .net.
//
//
// Author:
//   Andreia Gaita (avidigal@novell.com)
//
// Copyright 2007 Novell, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
-->


<xsl:stylesheet version="1.0" 
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns:svg="http://www.w3.org/2000/svg"
xmlns:xaml="http://schemas.microsoft.com/winfx/2006/xaml/presentation"
xmlns:msxsl="urn:schemas-microsoft-com:xslt"
exclude-result-prefixes="svg xsl xaml msxsl"

>
	<xsl:output method="xml" version="1.0" encoding="UTF-8" indent="yes"/>

	<!-- START HERE -->
	
	<xsl:template match="/">
		<xsl:apply-templates />
	</xsl:template>
	
	<xsl:template match="svg:svg">
		<xsl:variable name="defaults">
			<defaults>
				<xsl:call-template name="attributes">
					<xsl:with-param name="node" select="."/>
					<xsl:with-param name="filter" select="'inherit'"/>
				</xsl:call-template>
			</defaults>
		</xsl:variable>

		<xsl:variable name="attribs">
			<defaults>
				<xsl:copy-of select="msxsl:node-set($defaults)/defaults/@*"/>
			</defaults>
		</xsl:variable>

		<xsl:variable name="local-attributes">
			<attributes>
				<xsl:call-template name="attributes">
					<xsl:with-param name="name" select="'Canvas'"/>
					<xsl:with-param name="node" select="."/>
					<xsl:with-param name="filter" select="'local'"/>
				</xsl:call-template>
			</attributes>
		</xsl:variable>
		<Canvas xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation">
			<xsl:copy-of select="msxsl:node-set($local-attributes)/attributes/@*"/>
			<xsl:for-each select="msxsl:node-set($defaults)/defaults/*">
				<xsl:element name="Canvas.{local-name(.)}">
					<xsl:copy-of select="@*"/>
					<xsl:copy-of select="child::*"/>
				</xsl:element>
			</xsl:for-each>
			
			<xsl:copy-of select="msxsl:node-set($local-attributes)/attributes/child::*"/>
			<xsl:apply-templates>
					<xsl:with-param name="defaults" select="msxsl:node-set($attribs)"/>
			</xsl:apply-templates>

		</Canvas>

	</xsl:template>


	<!-- A element - for links -->
	
	<xsl:template match="svg:a">
		<!-- inherited transformations -->
		<xsl:param name="transform"/>
		<!-- inherited defaults -->
		<xsl:param name="defaults"/>

		<xsl:apply-templates>
					<xsl:with-param name="transform" select="$transform"/>
					<xsl:with-param name="defaults" select="$defaults"/>
		</xsl:apply-templates>
	</xsl:template>

	<!-- g element - children inherit these values and transforms -->
	<xsl:template match="svg:g">
		<xsl:param name="transform"/>
		<xsl:param name="defaults"/>
		
		<!-- gather up the default values for the children to have -->
		<xsl:variable name="defs">
			<defaults>
				<!-- get the parent's defaults, if any -->
				<xsl:copy-of select="$defaults/defaults/@*"/>
				<xsl:call-template name="attributes">
					<xsl:with-param name="node" select="."/>
				</xsl:call-template>
				<xsl:copy-of select="$defaults/defaults/*"/>
			</defaults>
		</xsl:variable>

		<!-- get any transformations and store to send to the children -->
		<xsl:variable name="local-transform">
				<TransformGroup 
					xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation">		
					<xsl:call-template name="split">
						<xsl:with-param name="template" select="'transform'"/>
						<xsl:with-param name="str" select="@transform"/>
						<xsl:with-param name="separator"><xsl:value-of select="' '"/></xsl:with-param>
					</xsl:call-template>
					<xsl:if test="$transform">
						<xsl:copy-of select="$transform/xaml:TransformGroup/*"/>
					</xsl:if>
				</TransformGroup>
		</xsl:variable>

		<xsl:choose>

			<!-- if there are more than 1 children, it's a group transform, send the whole $transform value -->
			<xsl:when test="@transform and count(child::*) > 1">
					<xsl:apply-templates>
						<xsl:with-param name="transform" select="msxsl:node-set($local-transform)"/>
						<xsl:with-param name="defaults" select="msxsl:node-set($defs)"/>
					</xsl:apply-templates>	
			</xsl:when>

			<xsl:otherwise>
		
			<!-- if there is only one children or there is no transformation to apply, cut out the TransformGroup tag (it might be empty, which is good) -->
				<xsl:apply-templates>
					<xsl:with-param name="transform" select="msxsl:node-set($local-transform)/xaml:TransformGroup/*"/>
					<xsl:with-param name="defaults" select="msxsl:node-set($defs)"/>
				</xsl:apply-templates>			

			</xsl:otherwise>
	
		</xsl:choose>

	</xsl:template>


	<!-- drawing children - they're all the same, really -->
	<xsl:template match="svg:path | svg:circle | svg:ellipse | svg:rect | svg:line | svg:polyline | svg:polygon">
		<!-- inherited transformations -->
		<xsl:param name="transform"/>
		<!-- inherited defaults -->
		<xsl:param name="defaults"/>

		<xsl:variable name="name" select="local-name(.)"/>
		<xsl:element name="{msxsl:node-set($mappings)/mappings/mapping[@name=$name]/@value}" xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation">
			<xsl:if test="@d">
				<xsl:attribute name="Data"><xsl:value-of select="@d"/></xsl:attribute>
			</xsl:if>

			<!-- process the attributes and store them in a variable for later use -->
			<xsl:variable name="local-attributes">
				<attributes>
					<xsl:call-template name="attributes">
						<xsl:with-param name="name" select="msxsl:node-set($mappings)/mappings/mapping[@name=$name]/@value"/>
						<xsl:with-param name="node" select="."/>
					</xsl:call-template>
				</attributes>
			</xsl:variable>


			<!-- inherit parents attributes -->
			<xsl:copy-of select="$defaults/defaults/@*"/>
			<!-- output this node's attributes -->
			<xsl:copy-of select="msxsl:node-set($local-attributes)/xaml:attributes/@*"/>

			<xsl:if test="not(@fill) and not($defaults/defaults/@Fill) and not(msxsl:node-set($local-attributes)/xaml:attributes/@Fill)">
				<xsl:attribute name="Fill">#000</xsl:attribute>
			</xsl:if>

			<!-- check if there are transforms inherited from the parent and aggregate them all into one -->
			<xsl:choose>
				<xsl:when test="msxsl:node-set($local-attributes)/xaml:attributes/*[contains(local-name(.), 'Transform')]">
					<xsl:element name="{msxsl:node-set($mappings)/mappings/mapping[@name=$name]/@value}.RenderTransform" 
										xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation">
						<xsl:choose>
							<xsl:when test="$transform[local-name(.)='TransformGroup']">
								<TransformGroup 
									xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation">
									
									<xsl:copy-of select="$transform/*"/>
									<xsl:copy-of select="msxsl:node-set($local-attributes)/xaml:attributes/*[contains(local-name(.), 'Transform')]"/>
									
								</TransformGroup>
							</xsl:when>
							<xsl:otherwise>
								<xsl:copy-of select="$transform"/>
								<xsl:copy-of select="msxsl:node-set($local-attributes)/xaml:attributes/*[contains(local-name(.), 'Transform')]"/>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:element>
				</xsl:when>
				<xsl:otherwise>
					<xsl:copy-of select="$transform"/>
				</xsl:otherwise>
			</xsl:choose>

			<!-- output remaning attributes that got turned into elements and that were not outputted in the above group -->
			<xsl:copy-of select="msxsl:node-set($local-attributes)/xaml:attributes/*[not(contains(local-name(.), 'Transform'))]"/>

		</xsl:element>
	</xsl:template>

	<xsl:template match="svg:text">
		<!-- inherited transformations -->
		<xsl:param name="transform"/>
		<!-- inherited defaults -->
		<xsl:param name="defaults"/>
		<xsl:variable name="attributes">
			<attributes>
				<xsl:call-template name="attributes">
					<xsl:with-param name="node" select="."/>
				</xsl:call-template>
			</attributes>
		</xsl:variable>

		<TextBlock xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation">
			<xsl:copy-of select="msxsl:node-set($attributes)/attributes/@*[local-name(.) != 'Text']"/>
			<Run><xsl:value-of select="msxsl:node-set($attributes)/attributes/@Text"/></Run>
		</TextBlock>
	</xsl:template>


<!-- DEFAULT TEMPLATES
	These are just default implementations that don't really do anything. 
-->

	<xsl:template match="svg:defs | add_more_catches_here">
		<!-- inherited transformations -->
		<xsl:param name="transform"/>
		<!-- inherited defaults -->
		<xsl:param name="defaults"/>

		<xsl:apply-templates>
					<xsl:with-param name="transform" select="$transform"/>
					<xsl:with-param name="defaults" select="$defaults"/>
		</xsl:apply-templates>
	</xsl:template>


<!-- ######## HELPER TEMPLATES ####### -->

	<!--- maps svg names into xaml names -->
	<xsl:variable name="mappings">
		<mappings>
			<mapping name="path" value="Path"  type="element"/>
			<mapping name="text" value="Text"  type="element"/>
			<mapping name="ellipse" value="Ellipse" type="element"/>
			<mapping name="circle" value="Ellipse" type="element"/>
			
			<mapping name="stroke" value="Stroke" type="attribute"/>
			<mapping name="fill" value="Fill" type="attribute">
				<ignore>none</ignore>
			</mapping>
			<mapping name="fill" parent="text" value="Foreground" type="attribute"/>
			<mapping name="transform" value="RenderTransform" type="element" template="transform" prefixParent="true"/>
			<mapping name="viewBox" value="RenderTransform" type="element" template="viewBox" prefixParent="true" names="x y Width Height" filter="local"/>
			<mapping name="stroke-linejoin" value="StrokeLineJoin" type="attribute"/>
			<mapping name="stroke-width" value="StrokeThickness" type="attribute"/>
			<mapping name="font-size" value="FontSize" type="attribute"/>
			<mapping name="text-anchor" value="AlignmentX" type="attribute">
				<value name="middle" value="Centered"></value>
			</mapping>			
			<mapping name="x" value="Canvas.Left" type="attribute" filter="local"/>
			<mapping name="y" value="Canvas.Top" type="attribute" filter="local"/>
			<mapping name="x" value="X" parent="viewBox" type="attribute" filter="local" op="mul" opval="-1">
				<ignore>0</ignore>
			</mapping>
			<mapping name="y" value="X" parent="viewBox" type="attribute" filter="local" op="mul" opval="-1">
				<ignore>0</ignore>
			</mapping>
			<mapping name="rx" value="Width" type="attribute" filter="local"/>
			<mapping name="ry" value="Height" type="attribute" filter="local"/>
			<mapping name="r" type="attribute"/>
			<mapping name="cx" value="Canvas.Left" type="attribute" filter="local"/>
			<mapping name="cy" value="Canvas.Top" type="attribute" filter="local"/>
		</mappings>
	</xsl:variable>

	<!-- transform template - to turn transform="..." properties into Transform* elements -->
	<xsl:template name="transform">
		<xsl:param name="name"/>
		<xsl:param name="str"/>
		<xsl:param name="prefix"/>

		<!-- check what the separator is on the list of values to process -->
		<xsl:variable name="separator">
			<xsl:choose>
				<xsl:when test="contains($str, ',')">,</xsl:when>
				<xsl:otherwise> </xsl:otherwise>
			</xsl:choose>
		</xsl:variable>

		<!-- each transformation outputs slightly different nodes, so check one by one -->
		
		<xsl:if test="starts-with($str, 'translate')">
			<xsl:element name="{prefix}TranslateTransform" namespace="http://schemas.microsoft.com/winfx/2006/xaml/presentation">
			
				<xsl:call-template name="splitCoords">
					<xsl:with-param name="str" select="substring-before(substring-after($str, 'translate('), ')')"/>
					<xsl:with-param name="separator"><xsl:value-of select="string($separator)"/></xsl:with-param>
				</xsl:call-template>
		
			</xsl:element>
		</xsl:if>
		<xsl:if test="starts-with($str, 'rotate')">
			<xsl:element name="{prefix}RotateTransform" namespace="http://schemas.microsoft.com/winfx/2006/xaml/presentation">
				<xsl:attribute name="Angle"><xsl:value-of select="substring-before(substring-after($str, 'rotate('), ')')"/></xsl:attribute>
				<xsl:if test="../@rx">
					<xsl:attribute name="CenterX"><xsl:value-of select="../@rx"/></xsl:attribute>
				</xsl:if>
				<xsl:if test="../@ry">
					<xsl:attribute name="CenterY"><xsl:value-of select="../@ry"/></xsl:attribute>
				</xsl:if>
				<xsl:if test="../@r">
					<xsl:attribute name="CenterX"><xsl:value-of select="../@r"/></xsl:attribute>
					<xsl:attribute name="CenterY"><xsl:value-of select="../@r"/></xsl:attribute>
				</xsl:if>

			</xsl:element>
		</xsl:if>
		<xsl:if test="starts-with($str, 'scale')">
			<xsl:element name="{prefix}ScaleTransform" namespace="http://schemas.microsoft.com/winfx/2006/xaml/presentation">
				<xsl:call-template name="splitCoords">
					<xsl:with-param name="str" select="substring-before(substring-after($str, 'scale('), ')')"/>
					<xsl:with-param name="prefix" select="'Scale'"/>
					<xsl:with-param name="separator" select="string($separator)"/>
				</xsl:call-template>
			</xsl:element>
		</xsl:if>

	</xsl:template>

	<!-- template to process the viewBox property of the svg and turn it into properties of the canvas -->
	<xsl:template name="viewBox">
		<xsl:param name="mapping"/>
		<xsl:param name="str"/>
		<xsl:param name="prefix"/>		

		<xsl:variable name="attributes">
			<attributes>
				<xsl:call-template name="match-name-value">
					<xsl:with-param name="names"><xsl:value-of select="$mapping/@names"/></xsl:with-param>
					<xsl:with-param name="values"><xsl:value-of select="$str"/></xsl:with-param>
					<xsl:with-param name="separator" select="' '"/>
				</xsl:call-template>
			</attributes>
		</xsl:variable>
		<xsl:copy-of select="msxsl:node-set($attributes)/attributes/@Width"/>
		<xsl:copy-of select="msxsl:node-set($attributes)/attributes/@Height"/>
		
		<xsl:variable name="transform">
			<transform>
				<xsl:call-template name="attributes">
					<xsl:with-param name="node" select="msxsl:node-set($attributes)/attributes"/>
				</xsl:call-template>
			</transform>
		</xsl:variable>
		<xsl:if test="count(msxsl:node-set($transform)/transform/@*) > 0 or count(msxsl:node-set($transform)/transform/*) > 0">
			<xsl:element name="{$prefix}TranslateTransform" namespace="http://schemas.microsoft.com/winfx/2006/xaml/presentation">
				<xsl:copy-of select="msxsl:node-set($transform)/transform/@*"/>
				<xsl:copy-of select="msxsl:node-set($transform)/transform/*"/>
			</xsl:element>
		</xsl:if>
	</xsl:template>
	<!-- template to build attribute nodes with pair values separated by $separator with names separated by $separator -->
	<xsl:template name="match-name-value">
		<xsl:param name="names"/>
		<xsl:param name="values"/>
		<xsl:param name="separator"/>
	
		<xsl:choose>
			<xsl:when test="contains($names, $separator)">
				<xsl:attribute name="{substring-before($names, $separator)}"><xsl:value-of select="substring-before($values, $separator)"/></xsl:attribute>
			
				<xsl:call-template name="match-name-value">
					<xsl:with-param name="names" select="substring-after($names, $separator)"/>
					<xsl:with-param name="values" select="substring-after($values, $separator)"/>
					<xsl:with-param name="separator" select="$separator"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:otherwise>
				<xsl:attribute name="{$names}"><xsl:value-of select="$values"/></xsl:attribute>
			</xsl:otherwise>
		</xsl:choose>	
	</xsl:template>

	<xsl:template name="splitCoords">
		<xsl:param name="mapping"/>
		<xsl:param name="str"/>
		<xsl:param name="prefix"/>
		<xsl:param name="separator"/>
		<xsl:param name="n" select="1"/>
	
		<xsl:choose>
			<!--- stupid contains thinks an empty space is equal to no space at all... -->
			<xsl:when test="contains($str,$separator) and substring-after($str, $separator) != $str">
				<xsl:choose>
					<xsl:when test="$n = 1 and ((msxsl:node-set($mapping)/ignore != substring-before($str, $separator)) or not($mapping))">
						<xsl:attribute name="{$prefix}X"><xsl:value-of select="substring-before($str, $separator)"/></xsl:attribute>
					</xsl:when>
					<xsl:when test="$n = 2 and ((msxsl:node-set($mapping)/ignore != substring-before($str, $separator)) or not($mapping))">
						<xsl:attribute name="{$prefix}Y"><xsl:value-of select="substring-before($str, $separator)"/></xsl:attribute>
					</xsl:when>
					<xsl:when test="$n = 3 and ((msxsl:node-set($mapping)/ignore != substring-before($str, $separator)) or not($mapping))">
						<xsl:attribute name="{$prefix}Width"><xsl:value-of select="substring-before($str, $separator)"/></xsl:attribute>
					</xsl:when>
					<xsl:when test="$n = 4 and ((msxsl:node-set($mapping)/ignore != substring-before($str, $separator)) or not($mapping))">
						<xsl:attribute name="{$prefix}Height"><xsl:value-of select="substring-before($str, $separator)"/></xsl:attribute>
					</xsl:when>
				</xsl:choose>

				<!-- recurse again cutting out the part we've processed -->
				<xsl:call-template name="splitCoords">
					<xsl:with-param name="mapping" select="$mapping"/>
					<xsl:with-param name="str" select="substring-after($str, $separator)"/>
					<xsl:with-param name="prefix" select="$prefix"/>
					<xsl:with-param name="separator" select="$separator"/>
					<xsl:with-param name="n" select="$n + 1"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:otherwise>
				<!-- this is the last one, end recursion -->
				<xsl:choose>
					<xsl:when test="$n = 1 and ((msxsl:node-set($mapping)/ignore != $str) or not($mapping))">
						<xsl:attribute name="{$prefix}X"><xsl:value-of select="$str"/></xsl:attribute>
					</xsl:when>
					<xsl:when test="$n = 2 and ((msxsl:node-set($mapping)/ignore != $str) or not($mapping))">
						<xsl:attribute name="{$prefix}Y"><xsl:value-of select="$str"/></xsl:attribute>
					</xsl:when>
					<xsl:when test="$n = 3 and ((msxsl:node-set($mapping)/ignore != $str) or not($mapping))">
						<xsl:attribute name="{$prefix}Width"><xsl:value-of select="$str"/></xsl:attribute>
					</xsl:when>
					<xsl:when test="$n = 4 and ((msxsl:node-set($mapping)/ignore != $str) or not($mapping))">
						<xsl:attribute name="{$prefix}Height"><xsl:value-of select="$str"/></xsl:attribute>
					</xsl:when>
				</xsl:choose>
			</xsl:otherwise>
		</xsl:choose>

	</xsl:template>

	<!-- call a template by name -->
	<xsl:template name="process-template">
		<xsl:param name="mapping"/>
		<xsl:param name="name"/>
		<xsl:param name="template"/>
		<xsl:param name="str"/>
		<xsl:param name="prefix"/>
		
		<xsl:choose>
			<xsl:when test="$template = 'transform'">
				<xsl:call-template name="transform">
					<xsl:with-param name="name" select="$name"/>
					<xsl:with-param name="str" select="$str"/>
					<xsl:with-param name="prefix" select="$prefix"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:when test="$template = 'viewBox'">
				<xsl:call-template name="viewBox">
					<xsl:with-param name="mapping" select="$mapping"/>
					<xsl:with-param name="name" select="$name"/>
					<xsl:with-param name="str" select="$str"/>
					<xsl:with-param name="prefix" select="$prefix"/>
				</xsl:call-template>
			</xsl:when>
		</xsl:choose>

	</xsl:template>
	
	<!-- helper template to recursively split a string and call another template to output each component -->
	<xsl:template name="split">
		<xsl:param name="template"/>
		<xsl:param name="str"/>
		<xsl:param name="separator"/>
		<xsl:choose>
			<xsl:when test="contains($str,$separator)">
				<xsl:call-template name="process-template">
					<xsl:with-param name="template" select="$template"/>
					<xsl:with-param name="str" select="substring-before($str,$separator)"/>
				</xsl:call-template>
				<xsl:call-template name="split">
					<xsl:with-param name="template" select="$template"/>
					<xsl:with-param name="str" select="substring-after($str,$separator)"/>
					<xsl:with-param name="separator" select="$separator"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:otherwise>
				<xsl:call-template name="process-template">
					<xsl:with-param name="template" select="$template"/>
					<xsl:with-param name="str" select="$str"/>
				</xsl:call-template>

			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
	
		<!-- helper template to recursively process a name-value pair in $str separated by $separator, and 
			call the attributes template to turn it into an attribute -->
	<xsl:template name="name-value">
		<xsl:param name="str"/>
		<xsl:param name="separator"/>

		<!-- trim -->
		<xsl:variable name="input" select="normalize-space($str)"/>
		
		<xsl:choose>
			<!--- if there are more to process, recurse -->
			<xsl:when test="contains($input,$separator)">
				<xsl:variable name="attribute">
					<attribute>
						<xsl:attribute name="{substring-before(substring-before($input,$separator), ' ')}"><xsl:value-of select="substring-after(substring-before($input,$separator), ' ')"/></xsl:attribute>
					</attribute>
				</xsl:variable>

				<xsl:call-template name="attributes">
					<xsl:with-param name="node" select="msxsl:node-set($attribute)/attribute"/>
				</xsl:call-template>
				
				<!-- recursiveness, here we go -->
				<xsl:call-template name="name-value">
					<xsl:with-param name="str" select="substring-after($input,$separator)"/>
					<xsl:with-param name="separator" select="$separator"/>
				</xsl:call-template>
			</xsl:when>
			
			<!-- we're at the last one, stop here -->
			<xsl:otherwise>
				<xsl:variable name="attribute">
					<attribute>
						<xsl:attribute name="{substring-before($input, ' ')}"><xsl:value-of select="substring-after($input, ' ')"/></xsl:attribute>
					</attribute>
				</xsl:variable>
				<xsl:call-template name="attributes">
						<xsl:with-param name="node" select="msxsl:node-set($attribute)/attribute"/>
				</xsl:call-template>

			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
	<!-- transform attributes by finding their correspondence in the mappings table. If they're marked as elements or need
	specific templates to be processed, call those -->
	<xsl:template name="attributes">
		<xsl:param name="name"/>
		<xsl:param name="node"/>
		<xsl:param name="filter"/>
		<xsl:param name="parent"/>
		
		<xsl:variable name="prefix">
			<xsl:if test="$name"><xsl:value-of select="$name"/>.</xsl:if>
		</xsl:variable>
		
		<xsl:variable name="local-parent">		
			<xsl:choose>
				<xsl:when test="$parent"><xsl:value-of select="$parent"/>.</xsl:when>
				<xsl:otherwise><xsl:value-of select="local-name(.)"/></xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		
		<xsl:for-each select="$node/@*">
			<xsl:variable name="attname" select="local-name(.)"/>
			<xsl:variable name="mapping" select="msxsl:node-set($mappings)/mappings/mapping[@name=$attname]"/>
			
			<xsl:if test="not($filter) or ($mapping[@parent=$local-parent and @filter = $filter]) or ($mapping[not(@parent) and @filter = $filter]) or ($mapping[not(@filter)])">
				<xsl:choose>
				
					<!-- process attributes that need to be processed with a specific template (like transforms) -->
					<!-- there are certain attributes that have different mapping names depending on the parent, so check here -->
					<xsl:when test="$mapping[@parent=$local-parent and @template != '']">
						<xsl:call-template name="process-template">
							<xsl:with-param name="mapping" select="$mapping[@name=$attname and @parent=$local-parent and @template != '']"/>
							<xsl:with-param name="name" select="$attname"/>
							<xsl:with-param name="template" select="$mapping[@name=$attname and @parent=$local-parent and @template != '']/@template"/>
							<xsl:with-param name="str" select="."/>
							<xsl:with-param name="prefix" select="$prefix"/>
						</xsl:call-template>
					</xsl:when>
	
					<!-- process attributes that need to be processed with a specific template (like transforms) -->
					<xsl:when test="$mapping[@name=$attname and @template != '']">
						<xsl:call-template name="process-template">
							<xsl:with-param name="mapping" select="$mapping[@name=$attname and @template != '']"/>
							<xsl:with-param name="name" select="$attname"/>
							<xsl:with-param name="template" select="$mapping[@name=$attname and @template != '']/@template"/>
							<xsl:with-param name="str" select="."/>
							<xsl:with-param name="prefix" select="$prefix"/>
						</xsl:call-template>
					</xsl:when>
					
					<!-- process normal attributes -->
					<xsl:otherwise>
						<xsl:choose>
	
							<!-- first do the custom attributes -->
							<xsl:when test="$attname='r'">
								<xsl:if test="not(../@cx)">
									<xsl:attribute name="Canvas.Left"><xsl:value-of select=". * -1"/></xsl:attribute>
								</xsl:if>

								<xsl:if test="not(../@cy)">
									<xsl:attribute name="Canvas.Top"><xsl:value-of select=". * -1"/></xsl:attribute>
								</xsl:if>
								
								<xsl:attribute name="Width"><xsl:value-of select=". * 2"/></xsl:attribute>
								<xsl:attribute name="Height"><xsl:value-of select=". * 2"/></xsl:attribute>
							</xsl:when>
							<xsl:when test="$attname='rx'">
								<xsl:attribute name="Width"><xsl:value-of select=". * 2"/></xsl:attribute>
							</xsl:when>
							<xsl:when test="$attname='ry'">
								<xsl:attribute name="Height"><xsl:value-of select=". * 2"/></xsl:attribute>
							</xsl:when>
							<xsl:when test="$attname='cx'">
								<xsl:attribute name="Canvas.Left">
									<xsl:choose>
										<xsl:when test="../@rx"><xsl:value-of select=". - ../@rx"/></xsl:when>
										<xsl:otherwise><xsl:value-of select=". - ../@r"/></xsl:otherwise>
									</xsl:choose>
								</xsl:attribute>
							</xsl:when>
							<xsl:when test="$attname='cy'">
								<xsl:attribute name="Canvas.Top">
									<xsl:choose>
										<xsl:when test="../@ry"><xsl:value-of select=". - ../@ry"/></xsl:when>
										<xsl:otherwise><xsl:value-of select=". - ../@r"/></xsl:otherwise>
									</xsl:choose>
								</xsl:attribute>
							</xsl:when>
	
							<xsl:when test="$attname='style'">
								<xsl:variable name="str" select="translate(., ':', ' ')"/>
								<xsl:call-template name="name-value">
									<xsl:with-param name="str" select="$str"/>
									<xsl:with-param name="separator" select="';'"/>
								</xsl:call-template>
							</xsl:when>
	
	
							<!-- if it's not a custom one, check the mappings -->
							<!-- if there is a mapping for this attribute, with a corresponding parent, and the value is not marked to ignore -->
							<xsl:when test="$mapping[@name=$attname and @parent=$local-parent] and not($mapping[@name=$attname]/ignore = .)">
								<xsl:attribute name="{$mapping[@name=$attname and @parent=$local-parent]/@value}">
									<xsl:choose>
										<xsl:when test="$mapping[@name=$attname and @parent=$local-parent]/value[@name=current()]"><xsl:value-of select="$mapping[@name=$attname and @parent=$local-parent]/value[@name=current()]/@value"/></xsl:when>
										<xsl:otherwise>
											<xsl:call-template name="output-attribute-value">
												<xsl:with-param name="mapping" select="$mapping[@name=$attname and @parent=$local-parent]"/>
												<xsl:with-param name="value" select="."/>
											</xsl:call-template>										
										</xsl:otherwise>
									</xsl:choose>
								</xsl:attribute>
							</xsl:when>
							
							<!-- if there is a mapping for this attribute (ignore parent) and  the value is not marked to ignore -->
							<xsl:when test="$mapping[@name=$attname] and not($mapping[@name=$attname]/ignore = .)">
								<xsl:attribute name="{$mapping[@name=$attname]/@value}">
									<xsl:choose>
										<xsl:when test="$mapping[@name=$attname]/value[@name=current()]"><xsl:value-of select="$mapping[@name=$attname]/value[@name=current()]/@value"/></xsl:when>
										<xsl:otherwise>
											<xsl:call-template name="output-attribute-value">
												<xsl:with-param name="mapping" select="$mapping[@name=$attname]"/>
												<xsl:with-param name="value" select="."/>
											</xsl:call-template>
										</xsl:otherwise>
									</xsl:choose>
								</xsl:attribute>
							</xsl:when>
						</xsl:choose>
					</xsl:otherwise>
				</xsl:choose>

			</xsl:if>
		</xsl:for-each>	

		<!-- output any text value -->
		<xsl:if test="text() and msxsl:node-set($mappings)/mappings/mapping[@name=$local-parent]">
			<xsl:attribute name="{msxsl:node-set($mappings)/mappings/mapping[@name=$local-parent]/@value}"><xsl:value-of select="."/></xsl:attribute>
		</xsl:if>
		
	</xsl:template>

	<xsl:template name="output-attribute-value">
		<xsl:param name="mapping"/>
		<xsl:param name="value"/>
		
		<xsl:choose>
			<xsl:when test="$mapping/@op = 'mul'"><xsl:value-of select="$value * $mapping/@opval"/></xsl:when>
			<xsl:when test="$mapping/@op = 'add'"><xsl:value-of select="$value + $mapping/@opval"/></xsl:when>
			<xsl:when test="$mapping/@op = 'sub'"><xsl:value-of select="$value - $mapping/@opval"/></xsl:when>
			<xsl:when test="$mapping/@op = 'div'"><xsl:value-of select="$value div $mapping/@opval"/></xsl:when>
			<xsl:otherwise><xsl:value-of select="$value"/></xsl:otherwise>
		</xsl:choose>
	</xsl:template>
<!-- ######## END OF HELPER TEMPLATES ####### -->

</xsl:stylesheet>
