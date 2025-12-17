docker run \
    --name dojo \
    --privileged \
    -v "${DOJO_PATH}:/opt/pwn.college" \
    -v "${DATA_PATH}:/data" \
    -e DOJO_ENV=production \
    -e DOJO_HOST=synergy.mahaloz.re \
    -e VIRTUAL_HOST=synergy.mahaloz.re \
    -e LETSENCRYPT_HOST=synergy.mahaloz.re \
    -e INTERNET_FOR_ALL="True" \
    -p 2222:22 -p 80:80 -p 443:443 \
    -d \
    pwncollege/dojo
