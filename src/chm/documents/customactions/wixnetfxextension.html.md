---
title: WixNetfxExtension
layout: documentation
after: using_standard_customactions
---
  <h1>WixNetfxExtension</h1>

  <p>The <a href='netfx_xsd_index.htm'>WixNetfxExtension</a> includes a set of custom actions to compile native images using Ngen.exe. For an example, see <a href="ngen_managed_assemblies.htm">How To: NGen managed assemblies during installation</a>.</p>

  <h2>PackageGroups</h2>

  <p>The WixNetfxExtension includes packagegroups that make it easier to include .Net in your bundle.</p>

  <table cellspacing="0" cellpadding="4" class="style1" border="1">
    <tr>
      <td valign="top">
        <p><b>PackageGroup ID</b></p>
      </td>

      <td valign="top">
        <p><b>Description</b></p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NetFx40Web</p>
      </td>

      <td>
        <p>.Net Framework 4.0 Full web setup.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NetFx40Redist</p>
      </td>

      <td>
        <p>.Net Framework 4.0 Full standalone setup.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NetFx40ClientWeb</p>
      </td>

      <td>
        <p>.Net Framework 4.0 Client Profile web setup.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NetFx40ClientRedist</p>
      </td>

      <td>
        <p>.Net Framework 4.0 Client Profile standalone setup.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NetFx45Web</p>
      </td>

      <td>
        <p>.Net Framework 4.5 web setup.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NetFx45Redist</p>
      </td>

      <td>
        <p>.Net Framework 4.5 standalone setup.</p>
      </td>
    </tr>
  </table>

  <h2>Properties</h2>

  <p>The WixNetfxExtension also includes a set of properties that can be used to detect the presence of various versions of the .NET Framework, the .NET Framework SDK and the Windows SDK. For information on how to use these properties to verify the user's .NET Framework version at install time see <a href="check_for_dotnet.htm">How To: Check for .NET Framework Versions</a>.</p>

  <p>The following property is applicable to all versions of the .NET Framework:</p>

  <table cellspacing="0" cellpadding="4" class="style1" border="1">
    <tr>
      <td valign="top">
        <p><b>Property name</b></p>
      </td>

      <td valign="top">
        <p><b>Meaning</b></p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORKINSTALLROOTDIR</p>
      </td>

      <td>
        <p>Set to the root installation directory for all versions of the .NET Framework (%windir%\Microsoft.NET\Framework\).</p>
      </td>
    </tr>
  </table>

  <p>Here is a complete list of properties for the <b>.NET Framework 1.0</b> product family:</p>

  <table cellspacing="0" cellpadding="4" class="style1" border="1">
    <tr>
      <td valign="top">
        <p><b>Property name</b></p>
      </td>

      <td valign="top">
        <p><b>Meaning</b></p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK10</p>
      </td>

      <td>
        <p>Set to 3321-3705 if the .NET Framework 1.0 is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK10INSTALLROOTDIR</p>
      </td>

      <td>
        <p>Set to the installation directory for the .NET Framework 1.0 (%windir%\Microsoft.NET\Framework\v1.0.3705).</p>
      </td>
    </tr>
  </table>

  <p>Here is a complete list of properties for the <b>.NET Framework 1.1</b> product family:</p>

  <table cellspacing="0" cellpadding="4" class="style1" border="1">
    <tr>
      <td valign="top">
        <p><b>Property name</b></p>
      </td>

      <td valign="top">
        <p><b>Meaning</b></p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK11</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 1.1 is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK11_SP_LEVEL</p>
      </td>

      <td>
        <p>Indicates the service pack level for the .NET Framework 1.1.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK11INSTALLROOTDIR</p>
      </td>

      <td>
        <p>Set to the installation directory for the .NET Framework 1.1 (%windir%\Microsoft.NET\Framework\v1.1.4322).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK11_ZH_CN_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 1.1 Chinese (Simplified) language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK11_ZH_TW_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 1.1 Chinese (Traditional) language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK11_CS_CZ_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 1.1 Czech language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK11_DA_DK_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 1.1 Danish language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK11_NL_NL_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 1.1 Dutch language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK11_FI_FI_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 1.1 Finnish language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK11_FR_FR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 1.1 French language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK11_DE_DE_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 1.1 German language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK11_EL_GR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 1.1 Greek language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK11_HU_HU_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 1.1 Hungarian language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK11_IT_IT_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 1.1 Italian language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK11_JA_JP_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 1.1 Japanese language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK11_KO_KR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 1.1 Korean language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK11_NB_NO_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 1.1 Norwegian language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK11_PL_PL_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 1.1 Polish language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK11_PT_BR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 1.1 Portuguese (Brazil) language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK11_PT_PT_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 1.1 Portuguese (Portugal) language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK11_RU_RU_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 1.1 Russian language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK11_ES_ES_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 1.1 Spanish language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK11_SV_SE_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 1.1 Swedish language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK11_TR_TR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 1.1 Turkish language pack is installed (not set otherwise).</p>
      </td>
    </tr>
  </table>

  <p>Here is a complete list of properties for the <b>.NET Framework 2.0</b> product family:</p>

  <table cellspacing="0" cellpadding="4" class="style1" border="1">
    <tr>
      <td valign="top">
        <p><b>Property name</b></p>
      </td>

      <td valign="top">
        <p><b>Meaning</b></p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK20</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 2.0 is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK20_SP_LEVEL</p>
      </td>

      <td>
        <p>Indicates the service pack level for the .NET Framework 2.0.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK20INSTALLROOTDIR</p>
      </td>

      <td>
        <p>Set to the installation directory for the .NET Framework 2.0 (%windir%\Microsoft.NET\Framework\v2.0.50727).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK20INSTALLROOTDIR64</p>
      </td>

      <td>
        <p>Set to the installation directory for the 64-bit .NET Framework 2.0 (%windir%\Microsoft.NET\Framework64\v2.0.50727).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK20_ZH_CN_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 2.0 Chinese (Simplified) language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK20_ZH_TW_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 2.0 Chinese (Traditional) language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK20_CS_CZ_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 2.0 Czech language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK20_DA_DK_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 2.0 Danish language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK20_NL_NL_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 2.0 Dutch language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK20_FI_FI_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 2.0 Finnish language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK20_FR_FR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 2.0 French language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK20_DE_DE_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 2.0 German language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK20_EL_GR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 2.0 Greek language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK20_HU_HU_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 2.0 Hungarian language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK20_IT_IT_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 2.0 Italian language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK20_JA_JP_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 2.0 Japanese language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK20_KO_KR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 2.0 Korean language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK20_NB_NO_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 2.0 Norwegian language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK20_PL_PL_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 2.0 Polish language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK20_PT_BR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 2.0 Portuguese (Brazil) language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK20_PT_PT_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 2.0 Portuguese (Portugal) language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK20_RU_RU_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 2.0 Russian language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK20_ES_ES_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 2.0 Spanish language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK20_SV_SE_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 2.0 Swedish language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK20_TR_TR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 2.0 Turkish language pack is installed (not set otherwise).</p>
      </td>
    </tr>
  </table>

  <p>Here is a complete list of properties for the <b>.NET Framework 3.0</b> product family:</p>

  <table cellspacing="0" cellpadding="4" class="style1" border="1">
    <tr>
      <td valign="top">
        <p><b>Property name</b></p>
      </td>

      <td valign="top">
        <p><b>Meaning</b></p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK30</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.0 is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK30_SP_LEVEL</p>
      </td>

      <td>
        <p>Indicates the service pack level for the .NET Framework 3.0. This value will not exist until a service pack is installed.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK30INSTALLROOTDIR</p>
      </td>

      <td>
        <p>Set to the installation directory for the .NET Framework 3.0 (%windir%\Microsoft.NET\Framework\v3.0).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK30INSTALLROOTDIR64</p>
      </td>

      <td>
        <p>Set to the installation directory for the 64-bit .NET Framework 3.0 (%windir%\Microsoft.NET\Framework64\v3.0).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK30_ZH_CN_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.0 Chinese (Simplified) language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK30_ZH_TW_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.0 Chinese (Traditional) language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK30_CS_CZ_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.0 Czech language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK30_DA_DK_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.0 Danish language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK30_NL_NL_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.0 Dutch language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK30_FI_FI_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.0 Finnish language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK30_FR_FR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.0 French language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK30_DE_DE_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.0 German language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK30_EL_GR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.0 Greek language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK30_HU_HU_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.0 Hungarian language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK30_IT_IT_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.0 Italian language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK30_JA_JP_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.0 Japanese language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK30_KO_KR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.0 Korean language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK30_NB_NO_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.0 Norwegian language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK30_PL_PL_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.0 Polish language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK30_PT_BR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.0 Portuguese (Brazil) language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK30_PT_PT_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.0 Portuguese (Portugal) language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK30_RU_RU_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.0 Russian language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK30_ES_ES_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.0 Spanish language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK30_SV_SE_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.0 Swedish language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK30_TR_TR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.0 Turkish language pack is installed (not set otherwise).</p>
      </td>
    </tr>
  </table>

  <p>Here is a complete list of properties for the <b>.NET Framework 3.5</b> product family:</p>

  <table cellspacing="0" cellpadding="4" class="style1" border="1">
    <tr>
      <td valign="top">
        <p><b>Property name</b></p>
      </td>

      <td valign="top">
        <p><b>Meaning</b></p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.5 is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35_SP_LEVEL</p>
      </td>

      <td>
        <p>Indicates the service pack level for the .NET Framework 3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35INSTALLROOTDIR</p>
      </td>

      <td>
        <p>Set to the installation directory for the .NET Framework 3.5 (%windir%\Microsoft.NET\Framework\v3.5).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35INSTALLROOTDIR64</p>
      </td>

      <td>
        <p>Set to the installation directory for the 64-bit .NET Framework 3.5 (%windir%\Microsoft.NET\Framework64\v3.5).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35_ZH_CN_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.5 Chinese (Simplified) language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35_ZH_TW_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.5 Chinese (Traditional) language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35_CS_CZ_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.5 Czech language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35_DA_DK_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.5 Danish language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35_NL_NL_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.5 Dutch language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35_FI_FI_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.5 Finnish language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35_FR_FR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.5 French language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35_DE_DE_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.5 German language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35_EL_GR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.5 Greek language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35_HU_HU_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.5 Hungarian language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35_IT_IT_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.5 Italian language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35_JA_JP_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.5 Japanese language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35_KO_KR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.5 Korean language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35_NB_NO_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.5 Norwegian language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35_PL_PL_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.5 Polish language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35_PT_BR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.5 Portuguese (Brazil) language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35_PT_PT_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.5 Portuguese (Portugal) language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35_RU_RU_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.5 Russian language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35_ES_ES_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.5 Spanish language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35_SV_SE_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.5 Swedish language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35_TR_TR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.5 Turkish language pack is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35_CLIENT</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 3.5 client profile is installed (not set otherwise).</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK35_CLIENT_SP_LEVEL</p>
      </td>

      <td>
        <p>Indicates the service pack level for the .NET Framework 3.5 client profile.</p>
      </td>
    </tr>
  </table>

  <p>Here is a complete list of properties for the <b>.NET Framework 4.0</b> product family:</p>

  <table cellspacing="0" cellpadding="4" class="style1" border="1">

    <tr>
      <td valign="top">
        <p><b>Property name</b></p>
      </td>

      <td valign="top">
        <p><b>Meaning</b></p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULL</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 full is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULL_SERVICING_LEVEL</p>
      </td>

      <td>
        <p>Indicates the service pack level for the .NET Framework 4.0 full. This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULLINSTALLROOTDIR</p>
      </td>

      <td>
        <p>Set to the installation directory for the .NET Framework 4.0 full (%windir%\Microsoft.NET\Framework\v4.0). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULLINSTALLROOTDIR64</p>
      </td>

      <td>
        <p>Set to the installation directory for the 64-bit .NET Framework 4.0 full (%windir%\Microsoft.NET\Framework64\v4.0). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULL_AR_SA_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 full Arabic language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULL_ZH_CN_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 full Chinese (Simplified) language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULL_ZH_TW_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 full Chinese (Traditional) language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULL_CS_CZ_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 full Czech language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULL_DA_DK_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 full Danish language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULL_NL_NL_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 full Dutch language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULL_FI_FI_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 full Finnish language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULL_FR_FR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 full French language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULL_DE_DE_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 full German language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULL_EL_GR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 full Greek language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULL_HE_IL_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 full Hebrew language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULL_HU_HU_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 full Hungarian language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULL_IT_IT_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 full Italian language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULL_JA_JP_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 full Japanese language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULL_KO_KR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 full Korean language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULL_NB_NO_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 full Norwegian language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULL_PL_PL_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 full Polish language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULL_PT_BR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 full Portuguese (Brazil) language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULL_PT_PT_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 full Portuguese (Portugal) language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULL_RU_RU_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 full Russian language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULL_ES_ES_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 full Spanish language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULL_SV_SE_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 full Swedish language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40FULL_TR_TR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 full Turkish language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENT</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 client profile is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENT_SERVICING_LEVEL</p>
      </td>

      <td>
        <p>Indicates the service pack level for the .NET Framework 4.0 client profile. This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENTINSTALLROOTDIR</p>
      </td>

      <td>
        <p>Set to the installation directory for the .NET Framework 4.0 full (%windir%\Microsoft.NET\Framework\v4.0). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENTINSTALLROOTDIR64</p>
      </td>

      <td>
        <p>Set to the installation directory for the 64-bit .NET Framework 4.0 full (%windir%\Microsoft.NET\Framework64\v4.0). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENT_AR_SA_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 client Arabic language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENT_ZH_CN_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 client Chinese (Simplified) language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENT_ZH_TW_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 client Chinese (Traditional) language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENT_CS_CZ_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 client Czech language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENT_DA_DK_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 client Danish language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENT_NL_NL_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 client Dutch language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENT_FI_FI_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 client Finnish language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENT_FR_FR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 client French language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENT_DE_DE_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 client German language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENT_EL_GR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 client Greek language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENT_HE_IL_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 client Hebrew language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENT_HU_HU_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 client Hungarian language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENT_IT_IT_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 client Italian language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENT_JA_JP_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 client Japanese language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENT_KO_KR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 client Korean language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENT_NB_NO_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 client Norwegian language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENT_PL_PL_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 client Polish language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENT_PT_BR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 client Portuguese (Brazil) language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENT_PT_PT_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 client Portuguese (Portugal) language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENT_RU_RU_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 client Russian language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENT_ES_ES_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 client Spanish language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENT_SV_SE_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 client Swedish language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK40CLIENT_TR_TR_LANGPACK</p>
      </td>

      <td>
        <p>Set to #1 if the .NET Framework 4.0 client Turkish language pack is installed (not set otherwise). This property is available starting with WiX v3.5.</p>
      </td>
    </tr>

  </table>

  <p>Here is a complete list of properties for the <b>.NET Framework 4.5</b> product family:</p>

  <table cellspacing="0" cellpadding="4" class="style1" border="1">

    <tr>
      <td valign="top">
        <p><b>Property name</b></p>
      </td>

      <td valign="top">
        <p><b>Meaning</b></p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK45</p>
      </td>

      <td>
        <p>Set to Release number of the .NET Framework 4.5 if installed (not set otherwise). This property is available starting with WiX v3.6.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK45_AR_SA_LANGPACK</p>
      </td>

      <td>
        <p>set to Release number of the .NET Framework 4.5 Arabic language pack if installed (not set otherwise). This property is available starting with WiX v3.6.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK45ZH_CN_LANGPACK</p>
      </td>

      <td>
        <p>Set to Release number of the .NET Framework 4.5 Chinese (Simplified) language pack if installed (not set otherwise). This property is available starting with WiX v3.6.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK45ZH_TW_LANGPACK</p>
      </td>

      <td>
        <p>Set to Release number of the .NET Framework 4.5 Chinese (Traditional) language pack if installed (not set otherwise). This property is available starting with WiX v3.6.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK45CS_CZ_LANGPACK</p>
      </td>

      <td>
        <p>Set to Release number of the .NET Framework 4.5 Czech language pack if installed (not set otherwise). This property is available starting with WiX v3.6.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK45DA_DK_LANGPACK</p>
      </td>

      <td>
        <p>Set to Release number of the .NET Framework 4.5 Danish language pack if installed (not set otherwise). This property is available starting with WiX v3.6.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK45NL_NL_LANGPACK</p>
      </td>

      <td>
        <p>Set to Release number of the .NET Framework 4.5 Dutch language pack if installed (not set otherwise). This property is available starting with WiX v3.6.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK45FI_FI_LANGPACK</p>
      </td>

      <td>
        <p>Set to Release number of the .NET Framework 4.5 Finnish language pack if installed (not set otherwise). This property is available starting with WiX v3.6.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK45FR_FR_LANGPACK</p>
      </td>

      <td>
        <p>Set to Release number of the .NET Framework 4.5 French language pack if installed (not set otherwise). This property is available starting with WiX v3.6.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK45DE_DE_LANGPACK</p>
      </td>

      <td>
        <p>Set to Release number of the .NET Framework 4.5 German language pack if installed (not set otherwise). This property is available starting with WiX v3.6.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK45EL_GR_LANGPACK</p>
      </td>

      <td>
        <p>Set to Release number of the .NET Framework 4.5 Greek language pack if installed (not set otherwise). This property is available starting with WiX v3.6.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK45HE_IL_LANGPACK</p>
      </td>

      <td>
        <p>Set to Release number of the .NET Framework 4.5 Hebrew language pack if installed (not set otherwise). This property is available starting with WiX v3.6.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK45HU_HU_LANGPACK</p>
      </td>

      <td>
        <p>Set to Release number of the .NET Framework 4.5 Hungarian language pack if installed (not set otherwise). This property is available starting with WiX v3.6.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK45IT_IT_LANGPACK</p>
      </td>

      <td>
        <p>Set to Release number of the .NET Framework 4.5 Italian language pack if installed (not set otherwise). This property is available starting with WiX v3.6.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK45JA_JP_LANGPACK</p>
      </td>

      <td>
        <p>Set to Release number of the .NET Framework 4.5 Japanese language pack if installed (not set otherwise). This property is available starting with WiX v3.6.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK45KO_KR_LANGPACK</p>
      </td>

      <td>
        <p>Set to Release number of the .NET Framework 4.5 Korean language pack if installed (not set otherwise). This property is available starting with WiX v3.6.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK45NB_NO_LANGPACK</p>
      </td>

      <td>
        <p>Set to Release number of the .NET Framework 4.5 Norwegian language pack if installed (not set otherwise). This property is available starting with WiX v3.6.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK45PL_PL_LANGPACK</p>
      </td>

      <td>
        <p>Set to Release number of the .NET Framework 4.5 Polish language pack if installed (not set otherwise). This property is available starting with WiX v3.6.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK45PT_BR_LANGPACK</p>
      </td>

      <td>
        <p>Set to Release number of the .NET Framework 4.5 Portuguese (Brazil) language pack if installed (not set otherwise). This property is available starting with WiX v3.6.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK45PT_PT_LANGPACK</p>
      </td>

      <td>
        <p>Set to Release number of the .NET Framework 4.5 Portuguese (Portugal) language pack if installed (not set otherwise). This property is available starting with WiX v3.6.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK45RU_RU_LANGPACK</p>
      </td>

      <td>
        <p>Set to Release number of the .NET Framework 4.5 Russian language pack if installed (not set otherwise). This property is available starting with WiX v3.6.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK45ES_ES_LANGPACK</p>
      </td>

      <td>
        <p>Set to Release number of the .NET Framework 4.5 Spanish language pack if installed (not set otherwise). This property is available starting with WiX v3.6.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK45SV_SE_LANGPACK</p>
      </td>

      <td>
        <p>Set to Release number of the .NET Framework 4.5 Swedish language pack if installed (not set otherwise). This property is available starting with WiX v3.6.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK45TR_TR_LANGPACK</p>
      </td>

      <td>
        <p>Set to Release number of the .NET Framework 4.5 Turkish language pack if installed (not set otherwise). This property is available starting with WiX v3.6.</p>
      </td>
    </tr>

  </table>

  <p>Here is a complete list of properties for the <b>.NET Framework SDK</b> and <b>Windows SDK</b>:</p>

  <table cellspacing="0" cellpadding="4" class="style1" border="1">
    <tr>
      <td valign="top">
        <p><b>Property name</b></p>
      </td>

      <td valign="top">
        <p><b>Meaning</b></p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK11SDKDIR</p>
      </td>

      <td>
        <p>The location of the .NET Framework 1.1 SDK installation root.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>NETFRAMEWORK20SDKDIR</p>
      </td>

      <td>
        <p>The location of the .NET Framework 2.0 SDK installation root.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>WINDOWSSDKCURRENTVERSIONDIR</p>
      </td>

      <td>
        <p>The location of the currently active version of the Windows SDK.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>WINDOWSSDKCURRENTVERSION</p>
      </td>

      <td>
        <p>The version number of the currently active version of the Windows SDK.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>WINDOWSSDK60ADIR</p>
      </td>

      <td>
        <p>The location of the Windows SDK 6.0a installation root.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>WINDOWSSDK61DIR</p>
      </td>

      <td>
        <p>The location of the Windows SDK 6.1 installation root.</p>
      </td>
    </tr>

    <tr>
      <td valign="top">
        <p>WINDOWSSDK70ADIR</p>
      </td>

      <td>
        <p>The location of the Windows SDK 7.0a installation root. This property is available starting with WiX v3.5.</p>
      </td>
    </tr>
  </table>

  <h2>Using WixNetfxExtension Properties</h2>

  <p>To use the WixNetfxExtension properties in an MSI, use the following steps:</p>

  <ul>
    <li>Add PropertyRef elements for items listed above that you want to use in your MSI.</li>

    <li>Add the -ext &lt;path to WixNetfxExtension.dll&gt; command line parameter when calling light.exe to include the WixNetfxExtension in the MSI linking process.</li>
  </ul>For example:
  <pre>
&lt;PropertyRef Id="NETFRAMEWORK20" /&gt;
</pre>
