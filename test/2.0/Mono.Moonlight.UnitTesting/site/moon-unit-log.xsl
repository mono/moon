<?xml version="1.0" encoding="ISO-8859-1"?>
<html xsl:version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://www.w3.org/1999/xhtml">
	<head>
	</head>
	<body style="font-family:Arial;font-size:12pt;">
		<table style="border-collapse: collapse; border: solid 1px whitesmoke;">

			<tr style="border: 1px solid whitesmoke;">
				<xsl:choose>
					<xsl:when test='MoonLog/Totals/@Failed = "0"'> <xsl:attribute name="bgcolor">lightgreen</xsl:attribute> </xsl:when>
					<xsl:otherwise> <xsl:attribute name="bgcolor">red</xsl:attribute> </xsl:otherwise>
				</xsl:choose>
				<td>Total:</td>
				<td colspan="3"> 
					Passed: <xsl:value-of select="MoonLog/Totals/@Passed" />, 
					KnownIssue: <xsl:value-of select="MoonLog/Totals/@KnownIssue" />, 
					NotExecuted: <xsl:value-of select="MoonLog/Totals/@NotExecuted" />, 
					Failed: <xsl:value-of select="MoonLog/Totals/@Failed" />
				</td>
			</tr>


			<xsl:for-each select="MoonLog/Test">
				<tr style="border: 1px solid whitesmoke;">
						<xsl:choose>
							<xsl:when test='@Result = "Passed"'> <xsl:attribute name="bgcolor">#BBFFBB</xsl:attribute> </xsl:when>
							<xsl:when test='@Result = "Failed"'> <xsl:attribute name="bgcolor">#FFCCCC</xsl:attribute> </xsl:when>
						</xsl:choose>

					<td style="border: 1px solid whitesmoke;"> <xsl:value-of select="@Class"/> </td>
					<td style="border: 1px solid whitesmoke;"> <xsl:value-of select="@Name"/> </td>
					<td style="border: 1px solid whitesmoke;"> 
						<xsl:value-of select="@Result"/>
					</td>
					<td style="border: 1px solid whitesmoke;">
						<xsl:value-of select="@Message"/>
					</td>
				</tr>
			</xsl:for-each>

			<tr style="border: 1px solid whitesmoke;">
				<xsl:choose>
					<xsl:when test='MoonLog/Totals/@Failed = "0"'> <xsl:attribute name="bgcolor">lightgreen</xsl:attribute> </xsl:when>
					<xsl:otherwise> <xsl:attribute name="bgcolor">red</xsl:attribute> </xsl:otherwise>
				</xsl:choose>
				<td>Total:</td>
				<td colspan="3"> 
					Passed: <xsl:value-of select="MoonLog/Totals/@Passed" />, 
					KnownIssue: <xsl:value-of select="MoonLog/Totals/@KnownIssue" />, 
					NotExecuted: <xsl:value-of select="MoonLog/Totals/@NotExecuted" />, 
					Failed: <xsl:value-of select="MoonLog/Totals/@Failed" />
				</td>
			</tr>
		</table>
	</body>
</html>

