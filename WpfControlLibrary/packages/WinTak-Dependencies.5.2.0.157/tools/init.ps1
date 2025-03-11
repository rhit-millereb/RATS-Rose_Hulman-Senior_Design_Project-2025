param($installPath, $toolsPath, $package, $project)
function Update-SourceVersion
{
    Param ([string]$Version, [bool]$AddSdkVersion)
    $NewVersion = 'AssemblyVersion("' + $Version + '")';
    $NewFileVersion = 'AssemblyFileVersion("' + $Version + '")';
    foreach ($o in $input)
    {
        $takSdkVersionPattern = 'TakSdkVersion\("[0-9]+(\.([0-9]+|\*)){1,3}"\)'
        $takSdkVersion = 'TakSdkVersion("' + $version + '")';
        if ((Get-Content $o.FullName) -match 'TakSdkVersion')
        {
            Write-Host "Updating  '$($o.FullName)' -> $Version"
            (Get-Content $o.FullName) | ForEach-Object  { 
            % {$_ -replace $takSdkVersionPattern, $takSdkVersion }
            } | Out-File $o.FullName -encoding UTF8 -force
        }
        elseif ($AddSdkVersion)
        {
            Write-Host "Setting  '$($o.FullName)' -> $Version"
            Add-Content $o.FullName ('[assembly: WinTak.Framework.TakSdkVersion("' + $version + '")]')
        }
    }
}
function Update-AllAssemblyInfoFiles ( $version )
{
    $addSdkVersion = $False
    $searchPath = ""
    if ($project)
    {
        $addSdkVersion = $True
        $searchPath = [System.IO.Path]::GetDirectoryName($project.FullName)
    }
    Write-Host "Searching '$searchPath'"
    foreach ($file in "AssemblyInfo.cs", "AssemblyInfo.vb" )
    {
        get-childitem $searchPath -recurse |? {$_.Name -eq $file} | Update-SourceVersion $version $addSdkVersion ;
    }
}
Update-AllAssemblyInfoFiles "5.2.0.157"
