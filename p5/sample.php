<!DOCTYPE html>
<html>
    <head>
        <meta http-equiv="content-type" content="text/html; charset=utf-8" />
        <title>Sample</title>
    </head>
    <body>
        <pre>
        <?php
        session_start();
        $_SESSION["ctr"]++;
        if ($_REQUEST['username'] == 'boy') {
            $_SESSION['isLoggedIn'] = true;
        }
        print_r($_REQUEST);
        print_r($_SESSION);
        if ($_SESSION['isLoggedIn']) {
            echo "I AM LOGGED IN";
        }
        session_destroy();
        ?>
        </pre>
        <form action="sample.php" method="POST" accept-charset="utf-8">
            <input type="text" name="username" />
            <input type="password" name="password" />
            <input type="submit" name="submit" value="My submit btn"/>
        </form>
    </body>
</html>
