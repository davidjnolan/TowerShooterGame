$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$Port = 8765
$Prefix = "http://127.0.0.1:$Port/"
$TasksFile = Join-Path $Root "TowerShooter_Tasks.json"

$listener = New-Object System.Net.HttpListener
$listener.Prefixes.Add($Prefix)
$listener.Start()

Write-Host "Tower Shooter Task Board running at $Prefix"
Write-Host "Close this window or press Ctrl+C to stop the server."
Write-Host ""

Start-Process $Prefix

function Get-ContentType([string]$Path) {
    switch ([System.IO.Path]::GetExtension($Path).ToLowerInvariant()) {
        ".html" { return "text/html; charset=utf-8" }
        ".json" { return "application/json; charset=utf-8" }
        ".js"   { return "application/javascript; charset=utf-8" }
        ".css"  { return "text/css; charset=utf-8" }
        ".txt"  { return "text/plain; charset=utf-8" }
        default { return "application/octet-stream" }
    }
}

try {
    while ($listener.IsListening) {
        $context = $listener.GetContext()
        $request = $context.Request
        $response = $context.Response

        try {
            if ($request.HttpMethod -eq "POST" -and $request.Url.AbsolutePath -eq "/api/tasks") {
                $reader = New-Object System.IO.StreamReader($request.InputStream, $request.ContentEncoding)
                $body = $reader.ReadToEnd()
                $reader.Close()

                $data = $body | ConvertFrom-Json
                if ($null -eq $data.tasks) {
                    throw "Expected a project object containing a tasks array."
                }

                $temp = "$TasksFile.tmp"
                [System.IO.File]::WriteAllText($temp, $body, [System.Text.UTF8Encoding]::new($false))
                Move-Item -Force $temp $TasksFile

                $bytes = [System.Text.Encoding]::UTF8.GetBytes('{"ok":true}')
                $response.StatusCode = 200
                $response.ContentType = "application/json; charset=utf-8"
                $response.OutputStream.Write($bytes, 0, $bytes.Length)
                $response.Close()
                continue
            }

            $relative = $request.Url.AbsolutePath.TrimStart("/")
            if ([string]::IsNullOrWhiteSpace($relative)) {
                $relative = "index.html"
            }

            $target = Join-Path $Root $relative
            $fullTarget = [System.IO.Path]::GetFullPath($target)
            $fullRoot = [System.IO.Path]::GetFullPath($Root)

            if (-not $fullTarget.StartsWith($fullRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
                $response.StatusCode = 403
                $response.Close()
                continue
            }

            if (-not (Test-Path $fullTarget -PathType Leaf)) {
                $response.StatusCode = 404
                $response.Close()
                continue
            }

            $bytes = [System.IO.File]::ReadAllBytes($fullTarget)
            $response.StatusCode = 200
            $response.ContentType = Get-ContentType $fullTarget
            $response.ContentLength64 = $bytes.Length
            $response.OutputStream.Write($bytes, 0, $bytes.Length)
            $response.Close()
        }
        catch {
            $message = $_.Exception.Message
            $bytes = [System.Text.Encoding]::UTF8.GetBytes($message)
            $response.StatusCode = 500
            $response.ContentType = "text/plain; charset=utf-8"
            $response.OutputStream.Write($bytes, 0, $bytes.Length)
            $response.Close()
        }
    }
}
finally {
    $listener.Stop()
    $listener.Close()
}
