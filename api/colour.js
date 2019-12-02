const Pusher = require('pusher')

const pusher = new Pusher({
  appId: process.env.PUSHER_ANGEL_APP_ID,
  key: process.env.PUSHER_ANGEL_KEY,
  secret: process.env.PUSHER_ANGEL_SECRET,
  cluster:  process.env.PUSHER_ANGEL_CLUSTER,
  useTLS: true
})

module.exports = async (req, res) => {
    let inputColour = req.body.colour;
    let responseColour = req.body.colour;

    //if it's a hex colour we need to convert to rgb
    if (inputColour.substring(0,1) == "#") {
      responseColour = hexToRgb(inputColour)
    }

      try {
        await new Promise((resolve, reject) => {
          pusher.trigger
            ('angel', "new-colour", { responseColour },
            err => {
              if (err) return reject(err)
              resolve()
            }
          )
        }
        )

        res.status(200).send()
    } catch(e) {
        console.log(e.message)
        res.status(501).send()
    }

//https://stackoverflow.com/a/5624139
    function hexToRgb(hex) {
      var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
      let res = result ? {
        r: parseInt(result[1], 16),
        g: parseInt(result[2], 16),
        b: parseInt(result[3], 16)
      } : null;

      return `${res.r},${res.g},${res.b}`;
    }
}
